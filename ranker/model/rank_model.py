import torch
import torch.nn as nn
import torch.nn.functional as F

class RankNet(nn.Module):
   def __init__(self, input_size, num_layers=None, hidden_sizes=[256, 256, 256, 256, 256], k=10):
      super(RankNet, self).__init__()
      
      if num_layers is not None:
         hidden_sizes = hidden_sizes[:num_layers]
      
      layers = []
      prev_size = input_size
      
      layers.append(nn.Linear(prev_size, hidden_sizes[0]))
      layers.append(nn.BatchNorm1d(hidden_sizes[0]))
      layers.append(nn.ReLU())
      layers.append(nn.Dropout(0.2))
      prev_size = hidden_sizes[0]
      
      for hidden_size in hidden_sizes[1:]:
         layers.append(nn.Linear(prev_size, hidden_size))
         layers.append(nn.BatchNorm1d(hidden_size))
         layers.append(nn.ReLU())
         layers.append(nn.Dropout(0.2))
         prev_size = hidden_size
      

      layers.append(nn.Linear(prev_size, 1))
      layers.append(nn.Tanh())
      
      self.network = nn.Sequential(*layers)
      self.k = k
   
   def forward(self, x):
      # Reshape input if necessary (batch_size * num_docs * num_features)
      if len(x.shape) == 3:
         batch_size, num_docs, num_features = x.shape
         x = x.reshape(batch_size * num_docs, num_features)
      return self.network(x)
   
   def predict_scores(self, query_docs_features):
      """
      Predict scores for a set of documents for a query
      Args:
         query_docs_features: Tensor of shape (batch_size, m, p) or (m, p) 
                              where m is number of docs and p is number of features
      Returns:
         scores: Tensor of shape (batch_size, m) or (m,) containing predicted scores
      """
      # Get original shape
      original_shape = query_docs_features.shape
      if len(original_shape) == 3:
         batch_size, num_docs, num_features = original_shape
      else:
         num_docs, num_features = original_shape
         batch_size = 1
      
      # Forward pass
      scores = self.forward(query_docs_features)
      
      # Reshape back to original dimensions
      if len(original_shape) == 3:
         return scores.reshape(batch_size, num_docs)
      return scores.reshape(num_docs)
   
   def compute_ranking_loss(self, doc_features, reference_ranks):
      scores = self.predict_scores(doc_features)
      
      # Get valid indices (documents with rank >= 0)
      valid_indices = [i for i, rank in enumerate(reference_ranks) if rank >= 0]
      invalid_indices = [i for i, rank in enumerate(reference_ranks) if rank < 0]
      
      # Sort indices by their reference ranks
      sorted_indices = sorted(valid_indices, key=lambda i: reference_ranks[i])
      
      # Calculate pairwise ranking loss
      ranking_loss = 0.0
      num_pairs = 0
      
      # For each pair of documents where i should be ranked higher than j
      for i in range(len(sorted_indices)):
         for j in range(i + 1, len(sorted_indices)):
            idx_i = sorted_indices[i]
            idx_j = sorted_indices[j]
            
            rank_diff = reference_ranks[idx_j] - reference_ranks[idx_i]
            
            score_diff = scores[idx_i] - scores[idx_j]
            desired_diff = rank_diff * 0.2  

            pair_loss = torch.log(1 + torch.exp(-score_diff + desired_diff))
            ranking_loss += pair_loss
            num_pairs += 1
      
      # Average the ranking loss over all pairs
      if num_pairs > 0:
         ranking_loss = ranking_loss / num_pairs
      
      # Add penalty for valid documents to have positive scores
      if len(valid_indices) > 0:
         valid_scores = scores[valid_indices]
         valid_loss = torch.mean(torch.relu(-valid_scores)) 
      else:
         valid_loss = torch.tensor(0.0, device=scores.device)
      
      # Add penalty for invalid documents to have negative scores
      if len(invalid_indices) > 0:
         invalid_scores = scores[invalid_indices]
         invalid_loss = torch.mean(torch.relu(invalid_scores))  # Penalize scores > 0
      else:
         invalid_loss = torch.tensor(0.0, device=scores.device)
      
      if len(sorted_indices) > 0:
         top_k_ref = sorted_indices[:self.k]
         _, top_k_pred = torch.topk(scores, min(self.k, len(scores)))
         
         accuracy_loss = 0.0
         for pred_idx in top_k_pred:
            membership = torch.tensor(0.0, device=scores.device)
            for ref_idx in top_k_ref:
               if pred_idx == ref_idx:
                  membership += 1.0
            accuracy_loss -= membership
         
         accuracy_loss = accuracy_loss / self.k
      else:
         accuracy_loss = torch.tensor(0.0, device=scores.device)
      

      total_loss =  ranking_loss + 0.1 * valid_loss + 0.1 * invalid_loss + 0.5 * accuracy_loss
      
      return total_loss 