from ... import Tensor
from .. import Parameter
from .module import Module
from typing import Any, Optional


class _BatchNorm(Module):
    num_features: int = ...
    eps: float = ...
    momentum: float = ...
    affine: bool = ...
    track_running_stats: bool = ...
    weight: Parameter = ...
    bias: Parameter = ...

    def __init__(self, num_features: int, eps: float = ..., momentum: float = ..., affine: bool = ...,
                 track_running_stats: bool = ...) -> None: ...

    def reset_running_stats(self) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore


class BatchNorm1d(_BatchNorm): ...


class BatchNorm2d(_BatchNorm): ...


class BatchNorm3d(_BatchNorm): ...


class SyncBatchNorm(_BatchNorm):
    # TODO set process_group to the write type once torch.distributed is stubbed
    def __init__(self, num_features: int, eps: float = ..., momentum: float = ..., affine: bool = ...,
                 track_running_stats: bool = ..., process_group: Optional[Any] = ...) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore
