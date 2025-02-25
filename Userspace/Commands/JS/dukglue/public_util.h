#pragma once

#include "detail_traits.h" // for index_tuple/make_indexes
#include "dukexception.h"

// This file has some useful utility functions for users.
// Hopefully this saves you from wading through the implementation.

/**
 * @brief      Push a value onto the duktape stack.
 *
 * WARNING: THIS IS NOT "PROTECTED." If an error occurs when pushing (unlikely, but possible),
 *          the Duktape fatal error handler will be invoked (and the program will probably
 * terminate).
 *
 * @param      ctx    duktape context
 * @param[in]  val    value to push
 */
template<typename FullT>
void dukglue_push(duk_context* ctx, const FullT& val) {
    // ArgStorage has some static_asserts in it that validate value types,
    // so we typedef it to force ArgStorage<RetType> to compile and run the asserts
    typedef typename dukglue::types::ArgStorage<FullT>::type ValidateReturnType;

    using namespace dukglue::types;
    DukType<typename Bare<FullT>::type>::template push<FullT>(ctx, std::move(val));
}

template<typename T, typename... ArgTs>
void dukglue_push(duk_context* ctx, const T& arg, ArgTs... args) {
    dukglue_push(ctx, arg);
    dukglue_push(ctx, args...);
}

inline void dukglue_push(duk_context* ctx) {
    // no-op
}

/**
 * WARNING: THIS IS NOT "PROTECTED." If an error occurs while reading (which is possible if you
 * didn't explicitly check the type), the fatal Duktape error handler will be invoked, and the
 * program will probably abort.
 */
template<typename RetT>
void dukglue_read(duk_context* ctx, duk_idx_t arg_idx, RetT* out) {
    // ArgStorage has some static_asserts in it that validate value types,
    // so we typedef it to force ArgStorage<RetType> to compile and run the asserts
    typedef typename dukglue::types::ArgStorage<RetT>::type ValidateReturnType;

    using namespace dukglue::types;
    *out = DukType<typename Bare<RetT>::type>::template read<RetT>(ctx, arg_idx);
}

// methods

// leaves return value on stack
template<typename ObjT, typename... ArgTs>
void dukglue_call_method(duk_context* ctx,
                         const ObjT&  obj,
                         const char*  method_name,
                         ArgTs... args) {
    dukglue_push(ctx, obj);
    duk_get_prop_string(ctx, -1, method_name);

    if ( duk_check_type(ctx, -1, DUK_TYPE_UNDEFINED) ) {
        duk_error(ctx, DUK_ERR_REFERENCE_ERROR, "Method does not exist", method_name);
        return;
    }

    if ( !duk_is_callable(ctx, -1) ) {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "Property is not callable");
        return;
    }

    duk_swap_top(ctx, -2);
    dukglue_push(ctx, args...);
    duk_call_method(ctx, sizeof...(args));
}

namespace dukglue {
namespace detail {

template<typename ObjT, typename... ArgTs, size_t... Indexes>
void call_method_safe_helper(duk_context*            ctx,
                             const ObjT&             obj,
                             const char*             method_name,
                             std::tuple<ArgTs...>&&  tup,
                             index_tuple<Indexes...> indexes) {
    dukglue_call_method(ctx, obj, method_name, std::forward<ArgTs>(std::get<Indexes>(tup))...);
}

template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, duk_idx_t>::type
call_method_safe(duk_context* ctx) {
    ObjT*                        obj         = (ObjT*)duk_require_pointer(ctx, -3);
    const char*                  method_name = (const char*)duk_require_pointer(ctx, -2);
    typedef std::tuple<ArgTs...> TupleT;
    TupleT*                      tuple = (TupleT*)duk_require_pointer(ctx, -1);
    duk_pop_3(ctx);

    call_method_safe_helper(ctx,
                            *obj,
                            method_name,
                            std::tuple<ArgTs...>(*tuple),
                            typename make_indexes<ArgTs...>::type());
    return 1;
}

template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, duk_idx_t>::type
call_method_safe(duk_context* ctx) {
    ObjT*                        obj         = (ObjT*)duk_require_pointer(ctx, -4);
    const char*                  method_name = (const char*)duk_require_pointer(ctx, -3);
    typedef std::tuple<ArgTs...> TupleT;
    TupleT*                      tuple = (TupleT*)duk_require_pointer(ctx, -2);
    RetT*                        out   = (RetT*)duk_require_pointer(ctx, -1);
    duk_pop_n(ctx, 4);

    call_method_safe_helper(ctx,
                            *obj,
                            method_name,
                            std::tuple<ArgTs...>(*tuple),
                            typename make_indexes<ArgTs...>::type());
    dukglue_read(ctx, -1, out);
    return 1;
}

} // namespace detail
} // namespace dukglue

template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, RetT>::type
dukglue_pcall_method(duk_context* ctx, const ObjT& obj, const char* method_name, ArgTs... args) {
    std::tuple<ArgTs...> tuple(args...);

    duk_push_pointer(ctx, (void*)&obj);
    duk_push_pointer(ctx, (void*)method_name);
    duk_push_pointer(ctx, (void*)&tuple);

    duk_idx_t rc
        = duk_safe_call(ctx, &dukglue::detail::call_method_safe<RetT, ObjT, ArgTs...>, 3, 1);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);

    duk_pop(ctx); // remove result from stack
}

template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, RetT>::type
dukglue_pcall_method(duk_context* ctx, const ObjT& obj, const char* method_name, ArgTs... args) {
    std::tuple<ArgTs...> tuple(args...);
    RetT                 out;

    duk_push_pointer(ctx, (void*)&obj);
    duk_push_pointer(ctx, (void*)method_name);
    duk_push_pointer(ctx, (void*)&tuple);
    duk_push_pointer(ctx, (void*)&out);

    duk_idx_t rc
        = duk_safe_call(ctx, &dukglue::detail::call_method_safe<RetT, ObjT, ArgTs...>, 4, 1);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);

    duk_pop(ctx); // remove result from stack
    return std::move(out);
}

// calls

// leaves return value on the stack
template<typename ObjT, typename... ArgTs>
void dukglue_call(duk_context* ctx, const ObjT& func, ArgTs... args) {
    dukglue_push(ctx, func);
    if ( !duk_is_callable(ctx, -1) ) {
        duk_pop(ctx);
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "Object is not callable");
        return;
    }

    dukglue_push(ctx, args...);
    duk_call(ctx, sizeof...(args));
}

// safe call
namespace dukglue {
namespace detail {

template<typename ObjT, typename... ArgTs, size_t... Indexes>
void call_safe_helper(duk_context*            ctx,
                      const ObjT&             obj,
                      std::tuple<ArgTs...>&&  tup,
                      index_tuple<Indexes...> indexes) {
    dukglue_call(ctx, obj, std::forward<ArgTs>(std::get<Indexes>(tup))...);
}

// leaves result on stack
template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, duk_ret_t>::type call_safe(duk_context* ctx) {
    ObjT*                        obj = (ObjT*)duk_require_pointer(ctx, -2);
    typedef std::tuple<ArgTs...> TupleT;
    TupleT*                      tuple = (TupleT*)duk_require_pointer(ctx, -1);
    duk_pop_2(ctx);

    call_safe_helper(ctx,
                     *obj,
                     std::tuple<ArgTs...>(*tuple),
                     typename make_indexes<ArgTs...>::type());
    return 1;
}

// leaves result on stack
// The result is read into RetT here because it can potentially trigger an error (with duk_error).
// If we did it "above" this function, that error would trigger a panic instead of error handling.
template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, duk_ret_t>::type call_safe(duk_context* ctx) {
    ObjT* obj = (ObjT*)duk_require_pointer(ctx, -3);

    typedef std::tuple<ArgTs...> TupleT;
    TupleT*                      tuple = (TupleT*)duk_require_pointer(ctx, -2);
    RetT*                        out   = (RetT*)duk_require_pointer(ctx, -1);
    duk_pop_3(ctx);

    call_safe_helper(ctx,
                     *obj,
                     std::tuple<ArgTs...>(*tuple),
                     typename make_indexes<ArgTs...>::type());
    dukglue_read(ctx, -1, out);
    return 1;
}

} // namespace detail
} // namespace dukglue

// Unlike duktape, this will remove the return value from the stack!
template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<std::is_void<RetT>::value, RetT>::type
dukglue_pcall(duk_context* ctx, const ObjT& obj, ArgTs... args) {
    std::tuple<ArgTs...> tuple(args...);

    duk_push_pointer(ctx, (void*)&obj);
    duk_push_pointer(ctx, (void*)&tuple);
    duk_int_t rc = duk_safe_call(ctx, &dukglue::detail::call_safe<RetT, ObjT, ArgTs...>, 2, 1);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);
    duk_pop(ctx); // remove result from stack
}

template<typename RetT, typename ObjT, typename... ArgTs>
typename std::enable_if<!std::is_void<RetT>::value, RetT>::type
dukglue_pcall(duk_context* ctx, const ObjT& obj, ArgTs... args) {
    std::tuple<ArgTs...> tuple(args...);
    RetT                 result;

    duk_push_pointer(ctx, (void*)&obj);
    duk_push_pointer(ctx, (void*)&tuple);
    duk_push_pointer(ctx, (void*)&result);
    duk_int_t rc = duk_safe_call(ctx, &dukglue::detail::call_safe<RetT, ObjT, ArgTs...>, 3, 1);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);

    duk_pop(ctx); // remove result from stack
    return std::move(result);
}

// peval
namespace dukglue {
namespace detail {

template<typename RetT>
duk_ret_t eval_safe(duk_context* ctx) {
    const char* str = (const char*)duk_require_pointer(ctx, -2);
    RetT*       out = (RetT*)duk_require_pointer(ctx, -1);
    duk_pop_2(ctx);

    duk_eval_string(ctx, str);
    dukglue_read(ctx, -1, out);
    return 1;
}

} // namespace detail
} // namespace dukglue

template<typename RetT>
typename std::enable_if<std::is_void<RetT>::value, RetT>::type dukglue_peval(duk_context* ctx,
                                                                             const char*  str) {
    int prev_top = duk_get_top(ctx);
    int rc       = duk_peval_string(ctx, str);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);

    duk_pop_n(ctx, duk_get_top(ctx) - prev_top); // pop any results
}

template<typename RetT>
typename std::enable_if<!std::is_void<RetT>::value, RetT>::type dukglue_peval(duk_context* ctx,
                                                                              const char*  str) {
    int prev_top = duk_get_top(ctx);

    RetT ret;
    duk_push_pointer(ctx, (void*)str);
    duk_push_pointer(ctx, (void*)&ret);
    int rc = duk_safe_call(ctx, &dukglue::detail::eval_safe<RetT>, 2, 1);
    if ( rc != 0 )
        throw DukErrorException(ctx, rc);
    duk_pop_n(ctx, duk_get_top(ctx) - prev_top); // pop any results
    return ret;
}

// register a global object (very simple helper, but very common for "Hello World"-ish applications)
template<typename T>
inline void dukglue_register_global(duk_context* ctx, const T& obj, const char* name) {
    dukglue_push(ctx, obj);
    duk_put_global_string(ctx, name);
}