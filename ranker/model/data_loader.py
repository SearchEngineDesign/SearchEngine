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
      Load data from JSON file
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
      # Load train data
      with open(self.file_path, 'r') as f:
         train_data = json.load(f)
      
      # Load test data
      with open('data/test_data.json', 'r') as f:
         test_data = json.load(f)
      
      # Process train data
      train_queries_data = []
      for query in train_data['queries']:
         features = []
         ranks = []
         doc_ids = []
         
         for doc in query['documents']:
            features.append(doc['features'])
            ranks.append(doc['rank'])
            doc_ids.append(doc['doc_id'])
         
         doc_features = torch.tensor(features, dtype=torch.float32)
         train_queries_data.append((doc_features, ranks, doc_ids))
      
      # Process test data
      test_queries_data = []
      for query in test_data['queries']:
         features = []
         ranks = []
         doc_ids = []
         
         for doc in query['documents']:
            features.append(doc['features'])
            ranks.append(doc['rank'])
            doc_ids.append(doc['doc_id'])
         
         doc_features = torch.tensor(features, dtype=torch.float32)
         test_queries_data.append((doc_features, ranks, doc_ids))
      
      self.train_data = train_queries_data
      self.test_data = test_queries_data
      
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
      
      # Split queries into train and test
      queries = dataset["queries"]
      train_queries, test_queries = train_test_split(
         queries, 
         test_size=self.test_size, 
         random_state=self.random_state
      )
      
      # Create train and test datasets
      train_dataset = {"queries": train_queries}
      test_dataset = {"queries": test_queries}
      
      # Save to files
      with open(output_file, 'w') as f:
         json.dump(train_dataset, f, indent=2) 
      
      with open('data/test_data.json', 'w') as f:
         json.dump(test_dataset, f, indent=2)