import torch
import torch.optim as optim
from rank_model import RankNet
from data_loader import RankingDataLoader
import argparse
import os
import matplotlib.pyplot as plt
import json
from datetime import datetime

def train_model(
   model: RankNet,
   train_data: list,
   test_data: list,
   num_epochs: int = 100,
   learning_rate: float = 0.001,
   batch_size: int = 32
):
   """
   Train the ranking model
   Args:
      model: The RankNet model
      train_data: List of tuples (doc_features, reference_ranks, doc_ids)
      num_epochs: Number of training epochs
      learning_rate: Learning rate for optimizer
      batch_size: Batch size for training
   """
   optimizer = optim.Adam(model.parameters(), lr=learning_rate)
   # Add learning rate scheduler
   scheduler = optim.lr_scheduler.ReduceLROnPlateau(
      optimizer, mode='min', factor=0.5, patience=5, verbose=True
   )
   
   for epoch in range(num_epochs):
      model.train()
      losses = []
      total_loss = 0
      num_queries = 0
      
      # Shuffle queries
      torch.manual_seed(epoch)  # For reproducibility
      indices = torch.randperm(len(train_data))
      
      for i in range(0, len(train_data), batch_size):
         batch_indices = indices[i:i + batch_size]
         batch_queries = [train_data[idx] for idx in batch_indices]
         
         batch_loss = 0
         for doc_features, reference_ranks, _ in batch_queries:
            loss = model.compute_ranking_loss(doc_features, reference_ranks)
            batch_loss += loss
         
         # Average loss over batch
         batch_loss = batch_loss / len(batch_queries)
         
         optimizer.zero_grad()
         batch_loss.backward()
         
         torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)
         
         optimizer.step()
         
         ## print(f"Batch {i} loss: {batch_loss.item()}")
         total_loss += batch_loss.item()
         num_queries += len(batch_queries)
         losses.append(batch_loss.item())
   
      avg_loss = total_loss / num_queries
      print(f"Epoch {epoch + 1}/{num_epochs}, Average Loss: {avg_loss:.4f}")
      
      scheduler.step(avg_loss)
      # plot_losses(losses, epoch)
      ndcg, accuracy, acc, _ = evaluate_model(model, test_data)
      print(f"Epoch {epoch + 1}/{num_epochs}, NDCG@{model.k} score: {ndcg:.4f}, Accuracy@{model.k} score: {accuracy:.4f}, Acc@{model.k} score: {acc:.4f}")

def plot_losses(losses, epoch):
   plt.plot(losses)
   plt.savefig(f'losses_{epoch}.png')

def evaluate_model(
   model: RankNet,
   test_data: list,
   output_dir: str = 'outputs'
):
   """
   Evaluate the model using NDCG@k and accuracy@k
   Args:
      model: The trained RankNet model
      test_data: List of tuples (doc_features, reference_ranks, doc_ids)
      output_dir: Directory to save evaluation results
   Returns:
      Tuple of (average NDCG@k score, average accuracy@k, predictions)
      where predictions is a list of lists of doc_ids
   """
   total_ndcg = 0
   total_accuracy = 0
   total_acc = 0
   predictions = []
   evaluation_results = []
   
   with torch.no_grad():
      for query_idx, (doc_features, reference_ranks, doc_ids) in enumerate(test_data):
         # Compute loss/NDCG
         loss = model.compute_ranking_loss(doc_features, reference_ranks)
         ndcg = 1 - loss.item()
         total_ndcg += ndcg
         
         # Get predictions
         scores = model.predict_scores(doc_features)
         _, top_k_indices = torch.topk(scores, model.k)
         top_k_doc_ids = [doc_ids[i] for i in top_k_indices]
         predictions.append(top_k_doc_ids)
         
         # Get actual top k documents (those with rank >= 0 and rank < k) in order
         actual_top_k = [doc_ids[i] for i, rank in sorted(
            [(i, rank) for i, rank in enumerate(reference_ranks) if rank >= 0 and rank < model.k],
            key=lambda x: x[1]
         )]
         
         correct_positions = sum(1 for i, doc_id in enumerate(top_k_doc_ids) 
                               if i < len(actual_top_k) and doc_id == actual_top_k[i])
         correct = sum(1 for doc_id in top_k_doc_ids if doc_id in actual_top_k)
         accuracy = correct_positions / model.k
         total_accuracy += accuracy
         acc = correct / model.k
         total_acc += acc

         # Store evaluation results for this query
         query_result = {
            'query_id': query_idx,
            'documents': []
         }
         
         # Get ranks for all documents based on predicted scores
         # First, get the sorted indices based on predicted scores (descending order)
         sorted_indices = torch.argsort(scores, descending=True)
         # Create a mapping from doc_id to predicted rank
         doc_id_to_pred_rank = {doc_ids[i]: rank + 1 for rank, i in enumerate(sorted_indices)}
         
         # Get scores and ranks for all documents
         for doc_id, rank in zip(doc_ids, reference_ranks):
            query_result['documents'].append({
               'doc_id': doc_id,
               'true_rank': rank.item() if isinstance(rank, torch.Tensor) else rank,
               'predicted_rank': doc_id_to_pred_rank[doc_id]
            })
         
         evaluation_results.append(query_result)

   avg_ndcg = total_ndcg / len(test_data)
   avg_accuracy = total_accuracy / len(test_data)
   avg_acc = total_acc / len(test_data)
   print(f"NDCG@{model.k} score: {avg_ndcg:.4f}")
   print(f"Accuracy@{model.k} score: {avg_accuracy:.4f}")
   print(f"Acc@{model.k} score: {avg_acc:.4f}")

   # Save evaluation results to file
   os.makedirs(output_dir, exist_ok=True)
   timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
   output_file = os.path.join(output_dir, f'evaluation_results_{timestamp}.json')
   
   with open(output_file, 'w') as f:
      json.dump(evaluation_results, f, indent=2)
   
   print(f"Evaluation results saved to {output_file}")
   
   return avg_ndcg, avg_accuracy, avg_acc, predictions

def main():
   parser = argparse.ArgumentParser(description='Train and evaluate RankNet model')
   parser.add_argument('--input_file', type=str, default='data/data.json', help='Path to input JSON file')
   parser.add_argument('--output_dir', type=str, default='outputs', help='Directory to save outputs')
   parser.add_argument('--num_epochs', type=int, default=5, help='Number of training epochs')
   parser.add_argument('--learning_rate', type=float, default=0.0001, help='Learning rate')
   parser.add_argument('--batch_size', type=int, default=32, help='Batch size')
   parser.add_argument('--k', type=int, default=10, help='Number of top documents to consider')
   parser.add_argument('--num_layers', type=int, default=5, help='Number of hidden layers')
   parser.add_argument('--hidden_size', type=int, default=256, help='Size of hidden layers')
   parser.add_argument('--test_size', type=float, default=0.1, help='Proportion of test set')
   parser.add_argument('--create_example', action='store_true', help='Create example dataset')
   
   args = parser.parse_args()
   
   # Create output directory if it doesn't exist
   os.makedirs(args.output_dir, exist_ok=True)
   
   # Create example dataset if requested
   if args.create_example:
      data_loader = RankingDataLoader(args.input_file)
      data_loader.create_example_dataset(
         args.input_file,
         num_queries=1000,
         num_docs_per_query=50,
         num_features=10,
         top_k=args.k
      )
      print(f"Created example dataset at {args.input_file}")
   
   # Load data
   data_loader = RankingDataLoader(
      args.input_file,
      test_size=args.test_size
   )
   train_data, test_data = data_loader.load_data()
   
   # Get input size from first query
   num_docs, num_features = train_data[0][0].shape
   
   # Initialize model
   model = RankNet(
      input_size=num_features,
      num_layers=args.num_layers,
      hidden_sizes=[args.hidden_size] * args.num_layers,
      k=args.k
   )
   
   # Train model
   train_model(
      model,
      train_data,
      test_data,
      num_epochs=args.num_epochs,
      learning_rate=args.learning_rate,
      batch_size=args.batch_size
   )
   
   # Evaluate model
   ndcg_score, accuracy, acc, predictions = evaluate_model(model, test_data, args.output_dir)
   print(f"NDCG@{args.k} score: {ndcg_score:.4f}")
   print(f"Accuracy@{args.k} score: {accuracy:.4f}")
   print(f"Acc@{args.k} score: {acc:.4f}")
   
   # Save the model in ONNX format
   # Create a dummy input with the same shape as the model expects
   dummy_input = torch.randn(1, num_docs, num_features)  # batch_size=1, num_docs, num_features
   
   onnx_path = os.path.join(args.output_dir, 'rank_model.onnx')
   torch.onnx.export(
      model,
      dummy_input,
      onnx_path,
      export_params=True,
      opset_version=11,
      do_constant_folding=True,
      input_names=['input'],
      output_names=['output'],
      dynamic_axes={
         'input': {0: 'batch_size', 1: 'num_docs'},  # batch and doc dimensions are dynamic
         'output': {0: 'batch_size', 1: 'num_docs'}
      }
   )
   print(f"Model saved in ONNX format to {onnx_path}")
   
   # # Save predictions
   # train_predictions = []
   # with torch.no_grad():
   #    for doc_features, _, doc_ids in train_data:
   #       scores = model.predict_scores(doc_features)
   #       _, top_k_indices = torch.topk(scores, args.k)
   #       top_k_doc_ids = [doc_ids[i] for i in top_k_indices]
   #       train_predictions.append(top_k_doc_ids)
   
   # # Save train and test predictions
   # data_loader.save_predictions(
   #    train_predictions,
   #    os.path.join(args.output_dir, 'train_predictions.json'),
   #    is_test=False
   # )
   # data_loader.save_predictions(
   #    predictions,
   #    os.path.join(args.output_dir, 'test_predictions.json'),
   #    is_test=True
   # )

if __name__ == "__main__":
   main() 