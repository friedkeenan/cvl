# cvl

A C++26 library for mutating consteval state.

An example of simple usage:

```cpp
#include <cvl/cvl.hpp>

/* A 'cvl::variable' models a mutable variable. */
constexpr cvl::variable example_var = 1;

/* We can dereference a 'cvl::variable' to get its value. */
static_assert(*example_var == 1);

consteval {
    /* We can assign to a 'cvl::variable' to set its value. */
    example_var = 2;
}

/* The value of the 'cvl::variable' has been updated. */
static_assert(*example_var == 2);
```

You can also use this library [in Compiler Explorer](https://compiler-explorer.com/z/sEscGE4f6) by using the following include:
```cpp
#include <https://raw.githubusercontent.com/friedkeenan/cvl/refs/heads/main/cvl_single_header.hpp>
```

This library is primarily for *exploratory* purposes, and is meant to provide tools for myself and others to probe and explore the powers and limits of mutable consteval state.

If you however wish to use this in a sincere project, then by all means feel free to do so. It would at the least be interesting to see how it might interact with a more mature codebase.

> [!NOTE]
>
> This library requires C++26 reflection.
>
> Make sure that your compiler supports this, and has the correct features enabled, if applicable.

## Features

`cvl` provides two main sets of features: Things which model mutable consteval state, and things which operate on that mutable consteval state.

***

The following entities within `cvl` model mutable state:

- `cvl::variable`
    - A `cvl::variable` models a mutable variable, and it can be updated an arbitrary number of times.
    - See its example [here](examples/variable.cpp).

- `cvl::list`
    - A `cvl::list` models a mutable list of values. It can only grow in size, and it can have an arbitrary number of elements.
    - See its example [here](examples/list.cpp).

- `cvl::delayed_init`
    - A `cvl::delayed_init` models a value whose initialization can be deferred to some point after its declaration.
    - See its example [here](examples/delayed_init.cpp).

- `cvl::once_flag`
    - A `cvl::once_flag` models a flag which can never be unset after it has been set at least once.
    - See its example [here](examples/once_flag.cpp).

***

The following entities within `cvl` operate on mutable state:

- `cvl::expand_loop`
    - With `cvl::expand_loop`, we are able to keep expanding a loop like with a `template for`, but crucially we can stop expanding based on any arbitrary, compile-time condition.
    - See its example [here](examples/expand_loop.cpp).
- `cvl::const_param`
    - A `cvl::const_param` allows us to lift a value passed as a function parameter into something we can use as a template parameter.
    - See its example [here](examples/const_param.cpp).

## Copying Mutable State

Every entity in `cvl` which models mutable consteval state has view-like semantics. That means that if we take a copy of an object of those types, then the original object and the copied object will both refer to the same value, and updating one will also update the other. For instance:

```cpp
constexpr cvl::variable original_var = 1;

constexpr auto copied_var = original_var;

/* The copy has the same value as the original. */
static_assert(*copied_var == 1);

consteval {
    /* Update the copied variable. */
    copied_var = 2;
}

/* The original variable has also now been updated. */
static_assert(*original_var == 2);
```

In order to "fork" a `cvl::variable` instead, then we would need to dereference the original variable, like so:

```cpp
constexpr cvl::variable original_var = 1;

constexpr cvl::variable forked_var = *original_var;
```

These two variables will then refer to distinct objects, and updating one would not update the other.

## Declaring Mutable State

The originating declaration of an entity within `cvl` that models mutable consteval state must also be a **static variable**. This means that you can define such objects as variables at namespace scope, as static variables inside a class, or as static variables inside a function. For instance:

```cpp
constexpr cvl::variable namespace_scoped = 1;

struct some_class {
    static constexpr cvl::variable class_scoped = 2;
};

auto some_function() -> void {
    static constexpr cvl::variable FunctionScoped = 3;
}
```

All these declarations are fine. What would not be fine would be something like the following:

```cpp
auto some_function() -> void {
    constexpr cvl::variable AutomaticStorage = 4;
}
```

Where `AutomaticStorage` (at least conceptually) lives on the stack, and does not have a static address. Attempting a declaration like this one will result in a compiler error.

We can, however, take copies of these objects without them needing to be static. For instance:

```cpp
auto some_function() -> void {
    static constexpr cvl::variable OriginalDeclaration = 5;

    constexpr cvl::variable CopiedDeclaration = OriginalDeclaration;
}
```

The declaration of `CopiedDeclaration` is allowed, because it is only referring to the same object as `OriginalDeclaration`, and is not declaring a new object.

## Order of Evaluation

In the use of this library, you may run into some issues pertaining to order of evaluation.

Sometimes, the compiler will try to outsmart us and evaluate our stateful consteval code at inopportune times for what we intend. This is a problem not just with `cvl`, but also with any other stateful consteval functionality, including for instance the standard `std::meta::is_complete_type` function.

This can manifest in several ways. For example, we can come across the following strange behavior:

```cpp
constexpr cvl::variable some_variable = 1;

template<typename T>
constexpr auto templated_function() -> int {
    constexpr int SomeValue = *some_variable;

    return SomeValue;
}

consteval {
    some_variable = 2;
}

/* 'templated_function' returns 1 even though we set 'some_variable' to 2! */
static_assert(templated_function<int>() == 1);
```

With this code, the compiler eagerly evaluates the initializing expression for `SomeValue`, before `templated_function` is ever instantiated, and initializes it with the value `1`. The compiler does this because it can see that initializing `SomeValue` doesn't involve any dependent names, so it decides that now's as good a time as any to evaluate it, and evaluates it right away.

We can get around this by introducing a dependent name, like so:

```cpp
template<typename T>
constexpr auto templated_function() -> int {
    constexpr int SomeValue = some_variable.dependent(^^T);

    return SomeValue;
}
```

This will then make the compiler wait to initialize `SomeValue` until it instantiates the template.

There is as well a related, and plausibly more insidious, issue with code like the following:

```cpp
constexpr cvl::variable some_variable = 1;

consteval auto consteval_function() -> int {
    const int some_value = *some_variable;

    return some_value;
}

consteval {
    some_variable = 2;
}

/* 'consteval_function' also returns 1 even though we set 'some_variable' to 2! */
static_assert(consteval_function() == 1);
```

Here, the compiler eagerly evaluates the initializing expression for  `some_value` before `consteval_function` is ever called. That is because the compiler is obligated to lift variables which are `const`, are of integral or enumeration type, and are initialized by a constant expression to being `constexpr` variables. So here it's actually as if we defined `some_value` like so:

```cpp
constexpr int some_value = *some_variable;
```

Which would much more clearly read as something which is eagerly evaluated. And indeed if we remove the `const` from `some_value`, then the compiler will evaluate `*some_variable` on each call to `consteval_function`, like we wanted. This is very spooky to me.

And, finally, there is another strangeness pertaining to evaluation order, found in code like the following:

```cpp
constexpr cvl::variable some_variable = 1;

int main() {
    std::printf("Some variable: %d\n", *some_variable);

    consteval {
        some_variable = 2;
    }
}
```

This code will print `Some variable: 2` even though it looks like we set `some_variable` to `2` *after* accessing it when calling `std::printf`. This happens because `consteval` blocks are evaluated when the compiler is building the "shape" of a function, the same as `constexpr` variables and `if constexpr` and `template for` and such. But the argument for `std::printf` is only evaluated *after* that "shaping" layer of the function is evaluated, after our `consteval` block has been evaluated.

There is a... weird way to get around this, where we could change the `consteval` block to an immediately-invoked lambda, like so:

```cpp
int main() {
    std::printf("Some variable: %d\n", *some_variable);

    []() {
        some_variable = 2;
    }();
}
```

And that will print `Some variable: 1`, but in my opinion the cleaner way to get around this issue is to explicitly pull out the variable's value into a `constexpr` variable, like so:

```cpp
int main() {
    constexpr auto Current = *some_variable;

    std::printf("Some variable: %d\n", Current);

    consteval {
        some_variable = 2;
    }
}
```

This will get evaluated in the right order, and makes it clear at what point values are being pulled out of a given variable.

## Compiler Support

This library is currently supported by GCC, which I am very grateful for.

However, due to a particular technique used in the implementation of this library, this library is unfortunately not currently supported by the experimental Clang reflection branch.
