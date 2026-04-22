


#include <cstddef>

#include <meta>
#include <compare>
#include <initializer_list>
#include <utility>
#include <optional>
#include <vector>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <type_traits>
#include <concepts>

#if defined(__GNUC__) and not defined(__clang__)

#define CVL_DISABLE_FRIEND_WARNING(tokens) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wnon-template-friend\"") \
    tokens \
    _Pragma("GCC diagnostic pop")

#else

#define CVL_DISABLE_FRIEND_WARNING(tokens) tokens

#endif

#if defined(__cpp_exceptions)

#define CVL_ERROR(what) throw ::std::meta::exception(what, ::std::meta::current_function())

#else

#define CVL_ERROR(what) ::cvl::impl::reached_error<std::define_static_string(what), ::std::meta::current_function()>()

#endif



namespace cvl {

    /*
        In order to support environments both
        with and without exceptions, we use
        slightly different erroring schemes
        for each, and conjoin their interface
        by using a macro defined in 'defines.hpp'.

        We apparently are effectively disallowed
        from inheriting from 'std::meta::exception'
        because we can't downgrade its destructor
        from 'consteval' to 'constexpr' but at
        the same time we can't define a 'consteval'
        destructor ourselves.

        So in the exception-supported environment,
        we just use 'std::meta::exception'.

        TODO: See if there's a better way than the above.
    */
    #if not defined(__cpp_exceptions)

    /*
        In the without-exceptions environment,
        we instead call a non-consteval function
        during constant evaluation, which will
        cause a compiler error.

        This non-consteval function has the
        description of the error passed to
        its template parameters, so we can
        still do our best to give relevant
        information to the user.
    */

    namespace impl {

        template<const char *What, std::meta::info From>
        auto reached_error() -> void;

    }

    #endif

}





namespace cvl::impl {

    /*
        We use friend injection in order to introduce
        our "consteval state", and since friend injection
        operates on function signatures, and thereby
        operates on *types*, we  thenneed to make sure
        that each "state" has its own unique type.

        We accomplish this by creating a "tag" that will
        be used as a template parameter to keep types
        distinct from each other.

        Tagged entities can inherit from 'impl::tagged' and
        conveniently and automatically get a unique tag,
        an object of type 'std::meta::info', which can then
        be passed off to various templates to get at the
        "backend" of a given state.
    */
    struct tagged {
        /*
            Tagged types operate like a view, like
            how 'std::meta::info' and pointers do.

            They in fact all refer to some other object,
            and operations on one tagged object will be
            reflected by all other tagged objects which
            refer to the same tag.
        */

        /*
            We get the constant of the 'this' pointer as our unique tag.

            This means that all originating-declarations (i.e. not copies)
            of an 'impl::tagged' object must have a static address.
        */
        std::meta::info _tag = std::meta::reflect_constant(this);

        /* A helper function to wrap our tag appropriately and send it off to the template. */
        consteval auto _substitute_tag(this const tagged self, const std::meta::info templ) -> std::meta::info {
            return substitute(templ, {
                std::meta::reflect_constant(self._tag)
            });
        }

        template<std::meta::reflection_range R = std::initializer_list<std::meta::info>>
        consteval auto _substitute_tag(this const tagged self, const std::meta::info templ, R &&args) -> std::meta::info {
            auto real_args = std::vector{std::meta::reflect_constant(self._tag)};

            real_args.append_range(std::forward<R>(args));

            return substitute(templ, real_args);
        }
    };

}


namespace cvl::util {

    namespace impl {

        /* Checking 'sizeof' forces the compiler to instantiate a templated type. */
        template<typename T>
        constexpr inline std::size_t ensure_type = sizeof(T);

        /* Getting the address of a function forces the compiler to instantiate a templated function. */
        template<std::meta::info Function>
        constexpr inline bool ensure_function = (&[: Function :] == nullptr);

    }

    /* Ensures that a template substitution gets instantiated by the compiler. */
    consteval auto ensure_instantiation(const std::meta::info entity) -> void {
        if (is_type(entity)) {
            (void) extract<std::size_t>(substitute(
                ^^impl::ensure_type, {entity}
            ));

            return;
        }

        if (not is_function(entity)) {
            return;
        }

        (void) extract<bool>(substitute(
            ^^impl::ensure_function, {std::meta::reflect_constant(entity)}
        ));
    }

    template<typename T>
    concept constant_reflectable = (
        std::is_copy_constructible_v<std::decay_t<T>> and

        std::is_structural_v<std::decay_t<T>>
    );

    template<typename T, typename Other>
    concept qualified_version_of = std::same_as<std::remove_cvref_t<T>, Other>;

}

namespace cvl {

    namespace impl {

        template<std::meta::info Tag>
        struct once_flag {
            CVL_DISABLE_FRIEND_WARNING(

                /*
                    This function returns a reflection of itself,
                    which is seemingly the only way we can get a
                    reflection of a friend function such as this one.

                    Later, 'impl::set_once_flag' will redeclare this
                    friend function, and will name the 'once_flag'
                    parameter, which we do not do here.

                    When that redeclaratiom occurs, the compiler will
                    then update whether that parameter has an identifier,
                    and that will change the result of 'std::meta::has_identifier'.

                    We can then check that to see whether
                    'impl::set_once_flag' has been instantiated.
                */
                friend consteval auto cvl_track_once_flag_set(once_flag) -> std::meta::info {
                    return std::meta::current_function();
                }

            )
        };

        template<std::meta::info Tag>
        struct set_once_flag {
            CVL_DISABLE_FRIEND_WARNING(

                /* Redeclare friend function, naming the parameter. */
                friend consteval auto cvl_track_once_flag_set(impl::once_flag<Tag> injected_name) -> std::meta::info;

            )
        };

        /* A helper template for us to call the friend function. */
        template<std::meta::info Tag>
        constexpr inline std::meta::info once_flag_tracker = cvl_track_once_flag_set(impl::once_flag<Tag>{});

    }

    struct once_flag : impl::tagged {
        consteval auto _tracker(this const once_flag self) -> std::meta::info {
            return extract<std::meta::info>(
                self._substitute_tag(^^impl::once_flag_tracker)
            );
        }

        consteval auto get(this const once_flag self) -> bool {
            const auto tracker = self._tracker();

            /* If the flag has been set, then the parameter will have an identifier. */
            return has_identifier(
                parameters_of(tracker)[0]
            );
        }

        consteval auto set(this const once_flag self) -> void {
            if (self.get()) {
                return;
            }

            const auto set_flag = self._substitute_tag(^^impl::set_once_flag);

            util::ensure_instantiation(set_flag);
        }
    };

}

namespace cvl {

    namespace impl {

        /* Tracks whether initialization has happened. */
        template<std::meta::info Tag>
        constexpr inline cvl::once_flag delayed_init_flag;

        template<std::meta::info Tag>
        struct delayed_init {
            CVL_DISABLE_FRIEND_WARNING(

                /* Will later have its definition filled in by 'impl::set_delayed_init_value'. */
                friend consteval auto cvl_delayed_init_value(delayed_init) -> auto;

            )
        };

        template<std::meta::info Tag, auto Value>
        struct set_delayed_init_value {
            CVL_DISABLE_FRIEND_WARNING(

                /* Filling in the function previously declared in 'impl::delayed_init'. */
                friend consteval auto cvl_delayed_init_value(impl::delayed_init<Tag>) -> auto {
                    return Value;
                }

            )

            consteval {
                /* Signal that initialization has taken place. */
                impl::delayed_init_flag<Tag>.set();
            }
        };

        /* Helper template to get the value for the 'delayed_init' tag. */
        template<std::meta::info Tag>
        constexpr inline auto delayed_init_value = cvl_delayed_init_value(impl::delayed_init<Tag>{});

    }

    template<util::constant_reflectable T>
    struct delayed_init : impl::tagged {
        consteval auto has_value(this const delayed_init self) -> bool {
            const auto flag = extract<cvl::once_flag>(
                self._substitute_tag(^^impl::delayed_init_flag)
            );

            return flag.get();
        }

        template<std::convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, delayed_init>)
        consteval auto operator =(this const delayed_init &self, Other &&value) -> const delayed_init & {
            if (self.has_value()) {
                CVL_ERROR("Cannot set value of 'cvl::delayed_init' which has already been initialized");
            }

            const auto set_value = self._substitute_tag(^^impl::set_delayed_init_value, {
                std::meta::reflect_constant(
                    static_cast<T>(std::forward<Other>(value))
                )
            });

            util::ensure_instantiation(set_value);

            return self;
        }

        consteval auto operator *(this const delayed_init self) -> const T & {
            if (not self.has_value()) {
                CVL_ERROR("Cannot use value of 'cvl::delayed_init' before it has been initialized");
            }

            return extract<const T &>(
                self._substitute_tag(^^impl::delayed_init_value)
            );
        }

        consteval auto operator ->(this const delayed_init self) -> const T * {
            return std::addressof(*self);
        }

        consteval auto optional(this const delayed_init self) -> std::optional<const T &> {
            if (not self.has_value()) {
                return std::nullopt;
            }

            return *self;
        }
    };

}

namespace cvl {

    namespace impl {

        template<typename Tag, typename T>
        struct const_param {
            static constexpr cvl::delayed_init<T> _value_holder = {};

            consteval const_param() = default;

            template<std::convertible_to<T> Other>
            requires (not cvl::util::qualified_version_of<Other, const_param>)
            consteval explicit(false) const_param(Other &&other) {
                /*
                    The compiler may call our constructor multiple times.

                    If that happens, we just return early.
                */
                if (_value_holder.has_value()) {
                    return;
                }

                _value_holder = std::forward<Other>(other);
            }

            consteval auto operator *(this const_param) -> const T & {
                if (not _value_holder.has_value()) {
                    CVL_ERROR("Cannot get value of 'cvl::const_param' before it's been passed a value");
                }

                return *_value_holder;
            }

            consteval auto operator ->(this const const_param self) -> const T * {
                return std::addressof(*self);
            }
        };

    }

    /*
        NOTE: We seemingly can't use the
        'cvl::impl::tagged' trick to uniquely
        tag a template parameter, so we fall
        back to using a lambda type as our tag.

        Having a default template parameter which
        doesn't have a consistent value may be
        an ODR violation however: https://stackoverflow.com/a/79289828

        If it is, then the compiler does not *seem* to care.

        But that is the primary reason why
        I am unsure about this utility.

        And I'm not sure if the constructor of
        'impl::const_param' *must* be evaluated
        before instantiating the function body.
    */
    template<util::constant_reflectable T, typename Tag = decltype([]{})>
    constexpr inline auto const_param = impl::const_param<Tag, T>{};

}


namespace cvl {

    namespace impl {

        struct loop_controller {
            cvl::once_flag _break_flag;

            /*
                Each loop iteration must be distinct,
                so we track the index of the loop.
            */
            std::size_t _index;

            consteval auto index(this const loop_controller self) -> std::size_t {
                return self._index;
            }

            consteval auto push_break(this const loop_controller self) -> void {
                self._break_flag.set();
            }
        };

        template<typename Body, std::meta::info... CallOperators>
        struct replicator {
            static constexpr auto execute(Body &body) -> void {
                template for (constexpr auto CallOperator : {CallOperators...}) {
                    body.[: CallOperator :]();
                }
            }
        };

        template<typename Body>
        consteval auto expand_loop() -> std::meta::info {
            auto args = std::vector{^^Body};

            static constexpr cvl::once_flag break_flag;

            std::size_t index = 0;

            while (true) {
                const auto controller = impl::loop_controller{break_flag, index};

                const auto call_operator = substitute(^^Body::template operator (), {
                    std::meta::reflect_constant(controller)
                });

                cvl::util::ensure_instantiation(call_operator);

                args.push_back(std::meta::reflect_constant(call_operator));

                if (break_flag.get()) {
                    break;
                }

                ++index;
            }

            return substitute(^^impl::replicator, args);
        }

    }

    /*
        NOTE: We call 'impl:;expand_loop' in the
        function interface to force the compiler
        to update state and such appropriately.

        TODO: It would be nice to have
        some requirements on 'Body' but
        that doesn't seem to work currently.
    */
    template<typename Body, typename Replicator = [: impl::expand_loop<std::remove_reference_t<Body>>() :]>
    constexpr auto expand_loop(Body &&body) -> void {
        Replicator::execute(body);
    }

    template<typename Body>
    constexpr auto expand_loop(Body &) -> void = delete("Loop bodies must be rvalues");

}


namespace cvl {

    namespace impl {

        /* Each element in a 'cvl::list' is stored here, at a particular index. */
        template<std::meta::info Tag, typename T, std::size_t Index>
        constexpr inline cvl::delayed_init<T> list_index_value;

        template<typename R, typename T>
        concept container_compatible_range = (
            std::ranges::input_range<R> and

            std::convertible_to<std::ranges::range_reference_t<R>, T>
        );

    }

    template<util::constant_reflectable T>
    requires (not std::is_reference_v<T>)
    struct list : impl::tagged, std::ranges::view_interface<list<T>> {
        struct iterator {
            /* Our iterator can be random access because it just maps to an index. */

            using iterator_concept  = std::random_access_iterator_tag;
            using iterator_category = std::random_access_iterator_tag;

            using reference       = const T &;
            using value_type      = T;
            using pointer         = const T *;
            using difference_type = std::ptrdiff_t;

            /* The tag from the 'cvl::list'. */
            std::meta::info _tag;

            /* The particular index that the iterator corresponds to. */
            std::ptrdiff_t  _index;

            consteval auto _index_value(this const iterator self) -> cvl::delayed_init<T> {
                return extract<cvl::delayed_init<T>>(
                    substitute(^^impl::list_index_value, {
                        std::meta::reflect_constant(self._tag),
                        ^^T,
                        std::meta::reflect_constant(
                            static_cast<std::size_t>(self._index)
                        )
                    })
                );
            }

            consteval auto operator *(this const iterator self) -> reference {
                const auto value = self._index_value();

                if (not value.has_value()) {
                    CVL_ERROR("Cannot access element of 'cvl::list' before it has been inserted");
                }

                return *value;
            }

            consteval auto operator ->(this const iterator self) -> pointer {
                return std::addressof(*self);
            }

            consteval auto operator ++(this iterator &self) -> iterator & {
                ++self._index;

                return self;
            }

            consteval auto operator ++(this iterator &self, int) -> iterator {
                const auto copy = self;

                ++self;

                return copy;
            }

            consteval auto operator --(this iterator &self) -> iterator & {
                --self._index;

                return self;
            }

            consteval auto operator --(this iterator &self, int) -> iterator {
                const auto copy = self;

                --self;

                return copy;
            }

            friend consteval auto operator ==(const iterator lhs, std::default_sentinel_t) -> bool {
                /* We've reached the end if the value for our index has not been initialized. */

                return not lhs._index_value().has_value();
            }

            /*
                We compare both our members for equality, since
                we need to compare against iterators from any range.
            */
            consteval auto operator ==(const iterator &) const -> bool = default;

            friend consteval auto operator <=>(const iterator lhs, const iterator rhs) -> auto {
                /*
                    We compare only our indices here, since
                    we only need to worry about relational
                    comparisons for iterators of the same range.
                */
                return (lhs._index <=> rhs._index);
            }

            friend consteval auto operator -(const iterator lhs, const iterator rhs) -> difference_type {
                return lhs._index - rhs._index;
            }

            friend consteval auto operator +=(iterator &lhs, const difference_type rhs) -> iterator & {
                lhs._index += rhs;

                return lhs;
            }

            friend consteval auto operator +(iterator lhs, const difference_type rhs) -> iterator {
                lhs += rhs;

                return lhs;
            }

            friend consteval auto operator +(const difference_type lhs, const iterator rhs) -> iterator {
                return rhs + lhs;
            }

            friend consteval auto operator -=(iterator &lhs, const difference_type rhs) -> iterator & {
                lhs._index -= rhs;

                return lhs;
            }

            friend consteval auto operator -(iterator lhs, const difference_type rhs) -> iterator {
                lhs -= rhs;

                return lhs;
            }

            consteval auto operator [](this iterator self, const difference_type index) -> reference {
                self += index;

                return *self;
            }
        };

        using value_type      = T;
        using reference       = const T &;
        using const_reference = reference;

        using const_iterator  = iterator;
        using difference_type = std::ptrdiff_t;

        consteval list() = default;

        template<impl::container_compatible_range<T> R>
        consteval list(std::from_range_t, R &&rng) {
            this->append_range(std::forward<R>(rng));
        }

        template<impl::container_compatible_range<T> R>
        requires (not util::qualified_version_of<R, list>)
        consteval explicit list(R &&rng) : list(std::from_range, std::forward<R>(rng)) {}

        consteval explicit(false) list(const std::initializer_list<T> rng) : list(std::from_range, rng) {}

        consteval auto begin(this const list self) -> iterator {
            return iterator{self._tag, 0};
        }

        consteval auto end(this list) -> std::default_sentinel_t {
            return std::default_sentinel;
        }

        consteval auto cbegin(this const list self) -> const_iterator {
            return self.begin();
        }

        consteval auto cend(this list) -> std::default_sentinel_t {
            return std::default_sentinel;
        }

        consteval auto _index_value(this const list self, const std::size_t index) -> cvl::delayed_init<T> {
            return extract<cvl::delayed_init<T>>(
                self._substitute_tag(^^impl::list_index_value, {
                    ^^T,
                    std::meta::reflect_constant(index)
                })
            );
        }

        consteval auto _next_index(this const list self) -> std::size_t {
            /* We loop until we find an index value which has not been initialized. */

            std::size_t index = 0;

            while (true) {
                const auto index_value = self._index_value(index);

                if (not index_value.has_value()) {
                    return index;
                }

                ++index;
            }
        }

        consteval auto push_back(this const list self, const T &elem) -> void {
            const auto next_index = self._next_index();

            self._index_value(next_index) = elem;
        }

        consteval auto try_back(this const list self) -> std::optional<const T &> {
            auto index = self._next_index();

            if (index <= 0) {
                return std::nullopt;
            }

            --index;

            return *self._index_value(index);
        }

        consteval auto try_at(this const list self, const std::size_t index) -> std::optional<const T &> {
            return self._index_value(index).optional();
        }

        template<impl::container_compatible_range<T> R>
        consteval auto append_range(this const list self, R &&rng) -> void {
            auto index = self._next_index();

            /* NOTE: 'elem' will have its lifetime extended if initialized by a temporary. */
            for (const T &elem : std::forward<R>(rng)) {
                self._index_value(index) = elem;

                ++index;
            }
        }

        [[nodiscard]]
        consteval auto empty(this const list self) -> bool {
            /* We are empty if the 0-th index has no value. */

            return not self._index_value(0).has_value();
        }

        friend consteval auto operator ==(const list lhs, const list rhs) -> bool {
            if (lhs._tag == rhs._tag) {
                return true;
            }

            return std::ranges::equal(lhs, rhs);
        }

        /* TODO: Is it worth implementing <=>? Probably. */
    };

    template<std::ranges::input_range R>
    list(std::from_range_t, R &&) -> list<std::ranges::range_value_t<R>>;

    template<std::ranges::input_range R>
    list(R &&) -> list<std::ranges::range_value_t<R>>;

}

namespace std::ranges {

    template<typename T>
    constexpr inline bool enable_borrowed_range<cvl::list<T>> = true;

}


namespace cvl {

    namespace impl {

        template<typename From, typename To>
        concept only_explicitly_convertible_to = not std::convertible_to<From, To> and requires {
            static_cast<To>(std::declval<From>());
        };

    }

    template<util::constant_reflectable T>
    requires (not std::is_reference_v<T>)
    struct variable {
        /*
            A 'cvl::variable' is built on top of
            a 'cvl::list', where each element in
            the list is a certain state of the variable.
        */
        cvl::list<T> _states;

        template<std::convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval explicit(false) variable(Other &&other) {
            this->states().push_back(std::forward<Other>(other));
        }

        template<impl::only_explicitly_convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval explicit variable(Other &&other) {
            this->states().push_back(
                static_cast<T>(std::forward<Other>(other))
            );
        }

        /*
            Because we use a 'cvl::list' to update our states,
            we can also just give access to that state list.
        */
        consteval auto states(this const variable self) -> cvl::list<T> {
            return self._states;
        }

        template<std::convertible_to<T> Other>
        requires (not util::qualified_version_of<Other, variable>)
        consteval auto operator =(this const variable &self, Other &&other) -> const variable & {
            self.states().push_back(std::forward<Other>(other));

            return self;
        }

        consteval auto operator *(this const variable self) -> const T & {
            /* The last state is our latest state. */
            const auto back = self.states().try_back();

            /* There will always be at least one element in the state list. */
            return *back;
        }

        consteval auto operator ->(this const variable self) -> const T * {
            return std::addressof(*self);
        }

        /* Used to explicitly access a variable as a dependent expression. */
        consteval auto dependent_value(this const variable self, auto &&) -> const T & {
            return *self;
        }
    };

    template<typename T>
    variable(T &&) -> variable<std::decay_t<T>>;

}

#undef CVL_DISABLE_FRIEND_WARNING

#undef CVL_ERROR
