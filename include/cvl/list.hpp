#pragma once

#include <cvl/common.hpp>
#include <cvl/delayed_init.hpp>
#include <cvl/tag.hpp>

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

    /* TODO: Add future requirements for 'cvl::delayed_init'. */
    template<typename T>
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

            consteval auto _index_value(this iterator self) -> cvl::delayed_init<T> {
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
        requires (not std::same_as<std::remove_cvref_t<R>, list>)
        consteval explicit list(R &&rng) : impl::tagged(), std::ranges::view_interface<list>() {
            this->append_range(std::forward<R>(rng));
        }

        consteval explicit(false) list(const std::initializer_list<T> rng) {
            this->append_range(rng);
        }

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
    list(R &&) -> list<std::ranges::range_value_t<R>>;

}

namespace std::ranges {

    template<typename T>
    constexpr inline bool enable_borrowed_range<cvl::list<T>> = true;

}
