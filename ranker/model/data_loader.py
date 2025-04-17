import torch
import json
import numpy as np
from typing import List, Tuple, Dict
from sklearn.model_selection import train_test_split

class RankingDataLoader:
   def __init__(self, file_path: str, test_size: float = 0.1, random_state: int = 42):
      """
      Initialize the data loader
      Args:
         file_path: Path to the input JSON file
         test_size: Proportion of dataset to include in the test split
         random_state: Random seed for reproducibility
      """
      self.file_path = file_path
      self.test_size = test_size
      self.random_state = random_state
      self.queries_data = None
      self.train_data = None
      self.test_data = None
   
   def load_data(self) -> Tuple[List[Tuple[torch.Tensor, List[int], List[str]]], 
                              List[Tuple[torch.Tensor, List[int], List[str]]]]:
      """
      Load and split data from JSON file
      Expected JSON format:
      {
         "queries": [
               {
                  "query_id": "1",
                  "documents": [
                     {
                           "doc_id": "1",
                           "features": [f1, f2, ..., fp],
                           "rank": 0  // -1 if not in top k
                     },
                     ...
                  ]
               },
               ...
         ]
      }
      Returns:
         Tuple of (train_data, test_data)
         Each is a list of tuples (doc_features, reference_ranks, doc_ids)
         where doc_features is tensor of shape (m, p)
         reference_ranks is list of length m
         doc_ids is list of length m
      """
      with open(self.file_path, 'r') as f:
         data = json.load(f)
      
      queries_data = []
      
      for query in data['queries']:
         # Extract features, ranks, and doc_ids
         features = []
         ranks = []
         doc_ids = []
         
         for doc in query['documents']:
               features.append(doc['features'])
               ranks.append(doc['rank'])
               doc_ids.append(doc['doc_id'])
         
         # Convert to tensors
         doc_features = torch.tensor(features, dtype=torch.float32)
         reference_ranks = ranks
         
         queries_data.append((doc_features, reference_ranks, doc_ids))
      
      self.queries_data = queries_data
      
      # Split into train and test
      train_indices, test_indices = train_test_split(
         range(len(queries_data)),
         test_size=self.test_size,
         random_state=self.random_state
      )
      
      self.train_data = [queries_data[i] for i in train_indices]
      self.test_data = [queries_data[i] for i in test_indices]
      
      return self.train_data, self.test_data

   def save_predictions(self, predictions: List[List[str]], output_file: str, is_test: bool = False):
      """
      Save predictions to JSON file
      Args:
         predictions: List of lists containing predicted doc_ids
         output_file: Path to save the predictions
         is_test: Whether these are test set predictions
      """
      with open(self.file_path, 'r') as f:
         data = json.load(f)
      
      # Get the appropriate data split
      data_split = self.test_data if is_test else self.train_data
      
      # Add predictions to the data
      for i, (_, _, doc_ids) in enumerate(data_split):
         query_id = data['queries'][i]['query_id']
         # Find the query in the original data
         for query in data['queries']:
               if query['query_id'] == query_id:
                  query['predictions'] = predictions[i]
                  break
      
      # Save to file
      with open(output_file, 'w') as f:
         json.dump(data, f, indent=2)

   def create_example_dataset(self, output_file: str, 
                           num_queries: int = 100,
                           num_docs_per_query: int = 50,
                           num_features: int = 10,
                           top_k: int = 10):
      """
      Create an example dataset with numeric IDs
      Args:
         output_file: Path to save the example dataset
         num_queries: Number of queries
         num_docs_per_query: Number of documents per query
         num_features: Number of features per document
         top_k: Number of top documents to rank
      """
      dataset = {
         "queries": []
      }
      
      # Set random seed for reproducibility
      np.random.seed(42)
      
      for i in range(1, num_queries + 1):  # Start from 1
         query = {
               "query_id": str(i),  # Convert to string for JSON
               "documents": []
         }
         
         # Generate random features
         features = np.random.randn(num_docs_per_query, num_features)
         
         # Generate random ranks (0 to top_k-1 for top k documents, -1 for others)
         ranks = [-1] * num_docs_per_query
         top_indices = np.random.choice(num_docs_per_query, top_k, replace=False)
         for j, idx in enumerate(top_indices):
               ranks[idx] = j
         
         # Create documents
         for j in range(1, num_docs_per_query + 1):  # Start from 1
               doc = {
                  "doc_id": str(j),  # Convert to string for JSON
                  "features": features[j-1].tolist(),
                  "rank": ranks[j-1]
               }
               query["documents"].append(doc)
         
         dataset["queries"].append(query)
      
      # Save to file
      with open(output_file, 'w') as f:
         json.dump(dataset, f, indent=2) 