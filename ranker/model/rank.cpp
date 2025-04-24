#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

class RankModel {
private:
   Ort::Env env;
   Ort::Session session{nullptr};
   std::vector<const char*> input_names;
   std::vector<const char*> output_names;
   Ort::AllocatorWithDefaultOptions allocator;

public:
   RankModel(const std::string& model_path) : env(ORT_LOGGING_LEVEL_WARNING, "RankModel") {
      // Create session options
      Ort::SessionOptions session_options;
      session_options.SetIntraOpNumThreads(1);
      session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

      // Load the ONNX model
      session = Ort::Session(env, model_path.c_str(), session_options);

      // Get input and output names
      Ort::AllocatorWithDefaultOptions allocator;
      input_names.push_back("input");
      output_names.push_back("output");
   }

   std::vector<float> predict(const std::vector<std::vector<float>>& doc_features) {
      // Convert input to tensor
      size_t batch_size = 1;
      size_t num_docs = doc_features.size();
      size_t num_features = doc_features[0].size();

      // Create input tensor
      std::vector<int64_t> input_shape = {static_cast<int64_t>(batch_size), 
                                       static_cast<int64_t>(num_docs), 
                                       static_cast<int64_t>(num_features)};
      
      // Flatten the input data
      std::vector<float> input_data;
      for (const auto& doc : doc_features) {
         input_data.insert(input_data.end(), doc.begin(), doc.end());
      }

      // Create input tensor
      auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
      Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
         memory_info, input_data.data(), input_data.size(), 
         input_shape.data(), input_shape.size());

      // Run inference
      auto output_tensors = session.Run(
         Ort::RunOptions{nullptr}, 
         input_names.data(), &input_tensor, 1, 
         output_names.data(), 1);

      // Get the output
      float* output_data = output_tensors[0].GetTensorMutableData<float>();
      size_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
      
      // Convert output to vector
      std::vector<float> predictions(output_data, output_data + output_size);
      return predictions;
   }

   std::vector<int> get_top_k(const std::vector<float>& scores, int k) {
      // Create vector of indices
      std::vector<int> indices(scores.size());
      for (size_t i = 0; i < scores.size(); ++i) {
         indices[i] = static_cast<int>(i);
      }

      // Sort indices based on scores in descending order
      std::sort(indices.begin(), indices.end(), 
               [&scores](int a, int b) { return scores[a] > scores[b]; });

      // Return top k indices
      return std::vector<int>(indices.begin(), indices.begin() + std::min(k, static_cast<int>(scores.size())));
   }
};

// Function to load test data from JSON file
std::vector<std::vector<std::vector<float>>> load_test_data(const std::string& file_path) {
    std::ifstream file(file_path);
    json data = json::parse(file);
    
    std::vector<std::vector<std::vector<float>>> all_queries_features;
    
    for (const auto& query : data["queries"]) {
        std::vector<std::vector<float>> query_features;
        for (const auto& doc : query["documents"]) {
            std::vector<float> features = doc["features"];
            query_features.push_back(features);
        }
        all_queries_features.push_back(query_features);
    }
    
    return all_queries_features;
}

int main() {
   try {
      // Initialize the model
      RankModel model("outputs/rank_model.onnx");

      // Load test data
      auto test_data = load_test_data("data/test_data.json");
      
      // Process each query
      for (size_t i = 0; i < test_data.size(); ++i) {
         std::cout << "Processing query " << i + 1 << std::endl;
         
         // Get predictions
         std::vector<float> scores = model.predict(test_data[i]);
         
         // Get top 10 documents
         int k = 10;
         std::vector<int> top_k = model.get_top_k(scores, k);
         
         // Print results
         std::cout << "Document scores: ";
         for (float score : scores) {
            std::cout << score << " ";
         }
         std::cout << std::endl;
         
         std::cout << "Top " << k << " documents: ";
         for (int idx : top_k) {
            std::cout << idx << " ";
         }
         std::cout << std::endl << std::endl;
      }

   } catch (const Ort::Exception& e) {
      std::cerr << "ONNX Runtime Error: " << e.what() << std::endl;
      return 1;
   } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }

   return 0;
}
