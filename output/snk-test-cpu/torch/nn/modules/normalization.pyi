from .module import Module
from typing import Any, Union, List
from ... import Tensor, Size
from .. import Parameter


class LocalResponseNorm(Module):
    size: int = ...
    alpha: float = ...
    beta: float = ...
    k: float = ...

    def __init__(self, size: int, alpha: float = ..., beta: float = ..., k: float = ...) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore


class CrossMapLRN2d(Module):
    size: int = ...
    alpha: float = ...
    beta: float = ...
    k: float = ...

    def __init__(self, size: int, alpha: float = ..., beta: float = ..., k: float = ...) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore


_shape_t = Union[int, List[int], Size]


class LayerNorm(Module):
    normalized_shape: _shape_t = ...
    eps: float = ...
    elementwise_affine: bool = ...
    weight: Parameter = ...
    bias: Parameter = ...

    def __init__(self, normalized_shape: _shape_t, eps: float = ..., elementwise_affine: bool = ...) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore


class GroupNorm(Module):
    num_groups: int = ...
    num_channels: int = ...
    eps: float = ...
    affine: bool = ...
    weight: Parameter = ...
    bias: Parameter = ...

    def __init__(self, num_groups: int, num_channels: int, eps: float = ..., affine: bool = ...) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore
