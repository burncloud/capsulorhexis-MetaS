#pragma once

#include "caffe2/opt/backend_transformer_base.h"

#include <unordered_set>

C10_DECLARE_bool(caffe2_tvm_profiling_based_jit);

namespace caffe2 {

struct TvmTransformOptions final : public BackendTransformOptions {
  explicit TvmTransformOptions() : BackendTransformOptions() {}

  //  Whether to enable profiling based jit
  bool profiling_based_jit{true};
};

class CAFFE2_API TvmTransformer final : public BackendTransformerBase {
 public:
  explicit TvmTransformer(const TvmTransformOptions& opts)
      : BackendTransformerBase(), opts_(opts) {}

  ~TvmTransformer() override {}

  // Given workspace and predict net, cluster continuous parts that can be run
  // by TVM and create one TVMJit op for each clustered subgraph. \param ws c2
  // workspace \param pred_net c2 predict net \param weight_names list of the
  // names of the constant weights \param shape_hints User provided shape info,
  // usually for primary inputs so that bound shape inference can have something
  // to start \param blacklisted_ops a set of ops that we don't want to lower to
  // TVM in terms of their net positions. This is very useful for debugging but
  // for normal runs it should be empty
  void transform(
      Workspace* ws,
      NetDef* pred_net,
      const std::vector<std::string>& weight_names,
      const std::unordered_map<std::string, TensorShape>& shape_hints,
      const std::unordered_set<int>& blacklisted_ops) override;

 private:
  // Given TVM runnable subnets, contract them into one TVMJitOp
  NetDef buildTvmOp(
      const caffe2::NetDef& net,
      const std::unordered_set<std::string>& weights,
      const ShapeInfoMap& shape_hints);

  // Apply transform to cluser connected TVM runnable ops into one TVMJitOp
  NetDef applyTvmTransform(
      NetDef* pred_net,
      const std::unordered_set<std::string>& weights,
      const std::unordered_set<int>& blacklisted_ops,
      const ShapeInfoMap& shape_hints);

  // Options
  TvmTransformOptions opts_;

  // Track number of TVMJitOp we created
  int tvm_op_id_{0};

  // Model id
  std::string model_id_;
};

// Helper function to clean up a net and run tvm transform.
CAFFE2_API void tvmTransform(
    NetDef* net,
    Workspace* ws,
    const std::vector<std::string>& input_names,
    const std::vector<std::string>& output_names,
    const std::vector<std::string>& weight_names,
    const std::unordered_map<std::string, TensorShape>& shape_hints,
    const std::unordered_set<int>& blacklisted_ops,
    size_t max_batch_size,
    size_t max_seq_size,
    bool debug);

} // namespace caffe2
