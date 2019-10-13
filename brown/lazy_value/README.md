## Lazy Value

The task is about lazy initialization.
Class template *LazyValue<T>* should construct object on demand only. It should accept a constructing functor in its constructor and use it only whe *Get* method is called.

Requirements for *LazyValue<T>*:
* constructor should not initialize T
* T should be constructed with the first call of *LazyValue<T>::Get*
* following calls of *LazyValue<T>::Get* should not reconstruct or change T
* T is not guaranteed to be default-constructible