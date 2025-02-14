//========================================================================
// parallel_reduce.inl
//========================================================================

namespace appl {

template <typename IndexT, typename ValueT, typename FuncT,
          typename ReduceT>
ValueT parallel_reduce( IndexT first, IndexT last, const ValueT initV,
                        const FuncT& func, const ReduceT& reduce ) {
  return func(first, last, initV);
}

} // namespace appl
