/*!
 * Copyright 2017 by Contributors
 * \file frontend.h
 * \brief Collection of front-end methods to load or construct ensemble model
 * \author Philip Cho
 */
#ifndef TREELITE_FRONTEND_H_
#define TREELITE_FRONTEND_H_

#include <treelite/base.h>
#include <memory>

namespace treelite {

struct Model;  // forward declaration

namespace frontend {

//--------------------------------------------------------------------------
// model loader interface: read from the disk
//--------------------------------------------------------------------------
/*!
 * \brief load a model file generated by LightGBM (Microsoft/LightGBM). The
 *        model file must contain a decision tree ensemble.
 * \param filename name of model file
 * \return loaded model
 */
Model LoadLightGBMModel(const char* filename);
/*!
 * \brief load a model file generated by XGBoost (dmlc/xgboost). The model file
 *        must contain a decision tree ensemble.
 * \param filename name of model file
 * \return loaded model
 */
Model LoadXGBoostModel(const char* filename);
/*!
 * \brief load a model in Protocol Buffers format. Protocol Buffers
 *        (google/protobuf) is a language- and platform-neutral mechanism for
 *        serializing structured data. See treelite.proto for format spec.
 * \param filename name of model file
 * \return loaded model
 */
Model LoadProtobufModel(const char* filename);

//--------------------------------------------------------------------------
// model builder interface: build trees incrementally
//--------------------------------------------------------------------------
struct ModelBuilderImpl;  // forward declaration

/*! \brief model builder class */
class ModelBuilder {
 public:
  ModelBuilder(int num_features);  // constructor
  ~ModelBuilder();  // destructor
  /*!
   * \brief Create a new tree
   * \param index location within the ensemble at which the new tree
   *              would be placed; use -1 to insert at the end
   * \return index of the new tree within the ensemble; -1 for failure
   */
  int CreateTree(int index = -1);
  /*!
   * \brief Remove a tree from the ensemble
   * \param index index of the tree that would be removed
   * \return whether successful
   */
  bool DeleteTree(int index);
  /*!
   * \brief Create an empty node within a tree
   * \param tree_index index of the tree into which the new node will be placed
   * \param node_key unique integer key to identify the new node
   * \return whether successful
   */
  bool CreateNode(int tree_index, int node_key);
  /*!
   * \brief Remove a node from a tree
   * \param tree_index index of the tree from which a node will be removed
   * \param node_key unique integer key to identify the node to be removed
   * \return whether successful
   */
  bool DeleteNode(int tree_index, int node_key);
  /*!
   * \brief Set a node as the root of a tree
   * \param tree_index index of the tree whose root is being set
   * \param node_key unique integer key to identify the root node
   * \return whether successful
   */
  bool SetRootNode(int tree_index, int node_key);
  /*!
   * \brief Turn an empty node into a test (non-leaf) node; the test is in the
   *        form [feature value] OP [threshold]. Depending on the result of the
   *        test, either left or right child would be taken.
   * \param tree_index index of the tree containing the node being modified
   * \param node_key unique integer key to identify the node being modified;
   *                 this node needs to be empty
   * \param feature_id id of feature
   * \param op binary operator to use in the test
   * \param threshold threshold value
   * \param default_left default direction for missing values
   * \param left_child_key unique integer key to identify the left child node
   * \param right_child_key unique integer key to identify the right child node
   * \return whether successful
   */
  bool SetTestNode(int tree_index, int node_key,
                   unsigned feature_id, Operator op, tl_float threshold,
                   bool default_left, int left_child_key, int right_child_key);
  /*!
   * \brief Turn an empty node into a leaf node
   * \param tree_index index of the tree containing the node being modified
   * \param node_key unique integer key to identify the node being modified;
   *                 this node needs to be empty
   * \param leaf_value leaf value (weight) of the leaf node
   * \return whether successful
   */
  bool SetLeafNode(int tree_index, int node_key, tl_float leaf_value);
  /*!
   * \brief finalize the model and produce the in-memory representation
   * \param out_model place to store in-memory representation of the finished
   *                  model
   * \return whether successful
   */
  bool CommitModel(Model* out_model);

 private:
  std::unique_ptr<ModelBuilderImpl> pimpl;  // Pimpl pattern
};

}  // namespace frontend
}  // namespace treelite
#endif  // TREELITE_FRONTEND_H_
