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
      """
      Compute the ranking loss for a single query
      Args:
          doc_features: Tensor of shape (num_docs, num_features)
          reference_ranks: List of reference ranks, where:
                         - rank = -1 for documents not in top 10
                         - rank = 1-10 for documents in top 10
      Returns:
          Loss value
      """
      # Get predictions
      scores = self.predict_scores(doc_features)
      
      # Calculate target scores based on ranks
      target_scores = torch.zeros_like(scores)
      for i, rank in enumerate(reference_ranks):
         if rank == -1:
            target_scores[i] = -1.0  # Target score for documents not in top 10
         else:
            target_scores[i] = 1.0 / rank  # Target score for documents in top 10
      
      loss = F.mse_loss(scores, target_scores)
      
      return loss 