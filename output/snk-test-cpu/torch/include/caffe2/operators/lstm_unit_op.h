#ifndef CAFFE2_OPERATORS_LSTM_UNIT_OP_H_
#define CAFFE2_OPERATORS_LSTM_UNIT_OP_H_

#include "caffe2/core/context.h"
#include "caffe2/core/operator.h"
#include "caffe2/utils/conversions.h"

namespace caffe2 {
namespace detail {
template <typename T>
inline T sigmoid(T x) {
  return 1. / (1. + exp(-x));
}

template <typename T>
inline T host_tanh(T x) {
  return 2. * sigmoid(2. * x) - 1.;
}

template <typename T, typename Context>
void LSTMUnit(
    int N,
    int D,
    int t,
    const T* H_prev,
    const T* C_prev,
    const T* X,
    const int32_t* seqLengths,
    bool drop_states,
    T* C,
    T* H,
    const float forget_bias,
    Context* /*context*/) {
  for (int n = 0; n < N; ++n) {
    const bool valid = seqLengths == nullptr || t < seqLengths[n];

    for (int d = 0; d < D; ++d) {
      if (!valid) {
        if (drop_states) {
          H[d] = 0;
          C[d] = 0;
        } else {
          H[d] = H_prev[d];
          C[d] = C_prev[d];
        }
      } else {
        const T i = sigmoid(X[d]);
        const T f = sigmoid(X[1 * D + d] + convert::To<float, T>(forget_bias));
        const T o = sigmoid(X[2 * D + d]);
        const T g = host_tanh(X[3 * D + d]);
        const T c_prev = C_prev[d];
        const T c = f * c_prev + i * g;
        C[d] = c;
        const T host_tanh_c = host_tanh(c);
        H[d] = o * host_tanh_c;
      }
    }
    H_prev += D;
    C_prev += D;
    X += 4 * D;
    C += D;
    H += D;
  }
}

template <typename T, typename Context>
void LSTMUnitGradient(
    int N,
    int D,
    int t,
    const T* C_prev,
    const T* X,
    const int32_t* seqLengths,
    const T* C,
    const T* H,
    const T* C_diff,
    const T* H_diff,
    bool drop_states,
    T* H_prev_diff,
    T* C_prev_diff,
    T* X_diff,
    const float forget_bias,
    Context* /*context*/) {
  for (int n = 0; n < N; ++n) {
    const bool valid = seqLengths == nullptr || t < seqLengths[n];

    for (int d = 0; d < D; ++d) {
      T* c_prev_diff = C_prev_diff + d;
      T* h_prev_diff = H_prev_diff + d;
      T* i_diff = X_diff + d;
      T* f_diff = X_diff + 1 * D + d;
      T* o_diff = X_diff + 2 * D + d;
      T* g_diff = X_diff + 3 * D + d;

      if (!valid) {
        if (drop_states) {
          *h_prev_diff = 0;
          *c_prev_diff = 0;
        } else {
          *h_prev_diff = H_diff[d];
          *c_prev_diff = C_diff[d];
        }
        *i_diff = 0;
        *f_diff = 0;
        *o_diff = 0;
        *g_diff = 0;
      } else {
        const T i = sigmoid(X[d]);
        const T f = sigmoid(X[1 * D + d] + convert::To<float, T>(forget_bias));
        const T o = sigmoid(X[2 * D + d]);
        const T g = host_tanh(X[3 * D + d]);
        const T c_prev = C_prev[d];
        const T c = C[d];
        const T host_tanh_c = host_tanh(c);
        const T c_term_diff = C_diff[d] + H_diff[d] * o * (1 - host_tanh_c * host_tanh_c);
        *c_prev_diff = c_term_diff * f;
        *h_prev_diff = 0; // not used in 'valid' case
        *i_diff = c_term_diff * g * i * (1 - i);
        *f_diff = c_term_diff * c_prev * f * (1 - f);
        *o_diff = H_diff[d] * host_tanh_c * o * (1 - o);
        *g_diff = c_term_diff * i * (1 - g * g);
      }
    }
    C_prev += D;
    X += 4 * D;
    C += D;
    H += D;
    C_diff += D;
    H_diff += D;
    X_diff += 4 * D;
    H_prev_diff += D;
    C_prev_diff += D;
  }
}
} // namespace detail

template <typename Context>
class LSTMUnitOp : public Operator<Context> {
 public:
  explicit LSTMUnitOp(const OperatorDef& operator_def, Workspace* ws)
      : Operator<Context>(operator_def, ws),
        forget_bias_(static_cast<float>(
            this->template GetSingleArgument<float>("forget_bias", 0.0))),
        sequence_lengths_(
            this->template GetSingleArgument<bool>("sequence_lengths", true)),
        drop_states_(
            this->template GetSingleArgument<bool>("drop_states", false)) {}
  USE_OPERATOR_CONTEXT_FUNCTIONS;
  using Operator<Context>::Operator;

  template <typename T>
  bool DoRunWithType() {
    // handle potentially-missing sequence lengths input
    const size_t TIMESTEP = SEQ_LENGTHS + (sequence_lengths_ ? 1 : 0);

    // Extract N
    const auto N = Input(CELL_T_M_1).size(1);

    // Gates: 1xNxG
    const auto G = Input(GATES).size(2);
    const auto D = Input(CELL_T_M_1).size(2);

    CAFFE_ENFORCE_EQ(4 * D, G);
    const auto* H_prev = Input(HIDDEN_T_M_1).template data<T>();
    const auto* C_prev = Input(CELL_T_M_1).template data<T>();
    const auto* X = Input(GATES).template data<T>();

    const int32_t* seqLengths = nullptr;
    if (sequence_lengths_) {
      CAFFE_ENFORCE_EQ(Input(SEQ_LENGTHS).numel(), N);
      seqLengths = Input(SEQ_LENGTHS).template data<int32_t>();
    }

    const auto t = static_cast<OperatorBase*>(this)
                       ->Input<Tensor>(TIMESTEP, CPU)
                       .template data<int32_t>()[0];
    Output(CELL_T)->ResizeLike(Input(CELL_T_M_1));
    auto* C = Output(CELL_T)->template mutable_data<T>();
    Output(HIDDEN_T)->ResizeLike(Input(CELL_T_M_1));
    auto* H = Output(HIDDEN_T)->template mutable_data<T>();
    detail::LSTMUnit<T, Context>(
        N,
        D,
        t,
        H_prev,
        C_prev,
        X,
        seqLengths,
        drop_states_,
        C,
        H,
        forget_bias_,
        &context_);
    return true;
  }

  bool RunOnDevice() override {
    return DoRunWithType<float>();
  }

 protected:
  INPUT_TAGS(HIDDEN_T_M_1, CELL_T_M_1, GATES, SEQ_LENGTHS);
  // additional input tags are determined dynamically based on whether
  // sequence_lengths is present.
  OUTPUT_TAGS(HIDDEN_T, CELL_T);

  float forget_bias_;
  bool sequence_lengths_;

 private:
  bool drop_states_;
};

template <typename Context>
class LSTMUnitGradientOp : public Operator<Context> {
 public:
  template <class... Args>
  explicit LSTMUnitGradientOp(Args&&... args)
      : Operator<Context>(std::forward<Args>(args)...),
        forget_bias_(static_cast<float>(
            this->template GetSingleArgument<float>("forget_bias", 0.0))),
        sequence_lengths_(
            this->template GetSingleArgument<bool>("sequence_lengths", true)),
        drop_states_(
            this->template GetSingleArgument<bool>("drop_states", false)) {}
  USE_OPERATOR_CONTEXT_FUNCTIONS;

  template <typename T>
  bool DoRunWithType() {
    // handle potentially-missing sequence lengths input
    const size_t inputOffset = SEQ_LENGTHS + (sequence_lengths_ ? 1 : 0);
    const size_t TIMESTEP = inputOffset;
    const size_t HIDDEN_T = inputOffset + 1;
    const size_t CELL_T = inputOffset + 2;
    const size_t HIDDEN_T_GRAD = inputOffset + 3;
    const size_t CELL_T_GRAD = inputOffset + 4;

    // Extract N
    const auto N = Input(CELL_T_M_1).size(1);

    // Gates: 1xNxG
    const auto G = Input(GATES).size(2);
    const auto D = Input(CELL_T_M_1).size(2);

    CAFFE_ENFORCE_EQ(4 * D, G);
    const auto* C_prev = Input(CELL_T_M_1).template data<T>();
    const auto* X = Input(GATES).template data<T>();
    const auto t = static_cast<OperatorBase*>(this)
                       ->Input<Tensor>(TIMESTEP, CPU)
                       .template data<int32_t>()[0];
    const auto* C = Input(CELL_T).template data<T>();
    const auto* H = Input(HIDDEN_T).template data<T>();
    const auto* C_diff = Input(CELL_T_GRAD).template data<T>();
    const auto* H_diff = Input(HIDDEN_T_GRAD).template data<T>();

    const int32_t* seqLengths = nullptr;
    if (sequence_lengths_) {
      CAFFE_ENFORCE_EQ(Input(SEQ_LENGTHS).numel(), N);
      seqLengths = Input(SEQ_LENGTHS).template data<int32_t>();
    }

    Output(HIDDEN_T_M_1_GRAD)->ResizeLike(Input(HIDDEN_T_M_1));
    auto* H_prev_diff = Output(HIDDEN_T_M_1_GRAD)->template mutable_data<T>();
    Output(CELL_T_M_1_GRAD)->ResizeLike(Input(CELL_T_M_1));
    auto* C_prev_diff = Output(CELL_T_M_1_GRAD)->template mutable_data<T>();
    Output(GATES_GRAD)->ResizeLike(Input(GATES));
    auto* X_diff = Output(GATES_GRAD)->template mutable_data<T>();

    detail::LSTMUnitGradient<T, Context>(
        N,
        D,
        t,
        C_prev,
        X,
        seqLengths,
        C,
        H,
        C_diff,
        H_diff,
        drop_states_,
        H_prev_diff,
        C_prev_diff,
        X_diff,
        forget_bias_,
        &context_);
    return true;
  }

  bool RunOnDevice() override {
    return DoRunWithType<float>();
  }

 protected:
  INPUT_TAGS(HIDDEN_T_M_1, CELL_T_M_1, GATES, SEQ_LENGTHS);
  // additional input tags are determined dynamically based on whether
  // sequence_lengths is present.
  OUTPUT_TAGS(HIDDEN_T_M_1_GRAD, CELL_T_M_1_GRAD, GATES_GRAD);

  float forget_bias_;
  bool sequence_lengths_;

 private:
  bool drop_states_;
};
} // namespace caffe2

#endif // CAFFE2_OPERATORS_LSTM_UNIT_OP_H_
