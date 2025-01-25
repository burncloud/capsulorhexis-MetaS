from ... import Tensor
from .module import Module
from .linear import Linear
from collections import namedtuple
from typing import List, Sequence
from .container import ModuleList

_ASMoutput = namedtuple('ASMoutput', ['output', 'loss'])


class AdaptiveLogSoftmaxWithLoss(Module):
    in_features: int = ...
    n_classes: int = ...
    cutoffs: List[int] = ...
    div_value: float = ...
    head_bias: bool = ...
    head: Linear = ...
    tail: ModuleList = ...

    def __init__(self, in_features: int, n_classes: int, cutoffs: Sequence[int], div_value: float = ...,
                 head_bias: bool = ...) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor, target: Tensor) -> _ASMoutput: ...  # type: ignore

    def __call__(self, input: Tensor, target: Tensor) -> _ASMoutput: ...  # type: ignore

    def log_prob(self, input: Tensor) -> List[float]: ...

    def predict(self, input: Tensor) -> Tensor: ...
