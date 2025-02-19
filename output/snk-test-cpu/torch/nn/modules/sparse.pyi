from .module import Module
from typing import Optional
from .. import Parameter
from ... import Tensor


class Embedding(Module):
    num_embeddings: int = ...
    embedding_dim: int = ...
    padding_idx: int = ...
    max_norm: float = ...
    norm_type: float = ...
    scale_grad_by_freq: bool = ...
    weight: Parameter = ...
    sparse: bool = ...

    def __init__(self, num_embeddings: int, embedding_dim: int, padding_idx: Optional[int] = ...,
                 max_norm: Optional[float] = ..., norm_type: float = ..., scale_grad_by_freq: bool = ...,
                 sparse: bool = ..., _weight: Optional[Tensor] = ...) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor) -> Tensor: ...  # type: ignore

    @classmethod
    def from_pretrained(cls, embeddings: Tensor, freeze: bool = ..., padding_idx: Optional[int] = ...,
                        max_norm: Optional[float] = ..., norm_type: float = ..., scale_grad_by_freq: bool = ...,
                        sparse: bool = ...): ...


class EmbeddingBag(Module):
    num_embeddings: int = ...
    embedding_dim: int = ...
    max_norm: float = ...
    norm_type: float = ...
    scale_grad_by_freq: bool = ...
    weight: Parameter = ...
    mode: str = ...
    sparse: bool = ...

    def __init__(self, num_embeddings: int, embedding_dim: int, max_norm: Optional[float] = ..., norm_type: float = ...,
                 scale_grad_by_freq: bool = ..., mode: str = ..., sparse: bool = ...,
                 _weight: Optional[Tensor] = ...) -> None: ...

    def reset_parameters(self) -> None: ...

    def forward(self, input: Tensor, offsets: Optional[Tensor] = ...) -> Tensor: ...  # type: ignore

    def __call__(self, input: Tensor, offsets: Optional[Tensor] = ...) -> Tensor: ...  # type: ignore

    @classmethod
    def from_pretrained(cls, embeddings: Tensor, freeze: bool = ..., max_norm: Optional[float] = ...,
                        norm_type: float = ..., scale_grad_by_freq: bool = ..., mode: str = ...,
                        sparse: bool = ...): ...
