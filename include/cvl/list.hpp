#pragma once

#include <cvl/common.hpp>
#include <cvl/map.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<typename R, typename T>
        concept container_compatible_range = (
            std::ranges::input_range<R> and

            std::convertible_to<std::ranges::range_reference_t<R>, T>
        );

    }

    template<util::constant_reflectable T>
    requires (not std::is_reference_v<T>)
    struct list : std::ranges::view_interface<list<T>> {
        struct iterator {
            /*
                Our iterator uses an 'iota_view' iterator internally
                to track which index the iterator corresponds to.

                This gives us some goodies like a better
                difference type, and simplifies some logic.

                We're able to use the iterator after its
                range has ended its lifetime, because
                'iota_view' models 'borrowed_range'.
            */
            using _internal_iterator = std::ranges::iterator_t<
                std::ranges::iota_view<std::size_t, std::unreachable_sentinel_t>
            >;

            /* Our iterator can be random access because it just maps to an index. */

            using iterator_concept  = std::random_access_iterator_tag;
            using iterator_category = std::random_access_iterator_tag;

            using reference       = const T &;
            using value_type      = T;
            using pointer         = const T *;
            using difference_type = std::iter_difference_t<_internal_iterator>;

            /* The map for the 'cvl::list'. */
            cvl::map<std::size_t, T> _map;

            /* The iterator corresponding to the current index. */
            _internal_iterator _current;

            consteval auto operator *(this const iterator self) -> reference {
                const auto value = self._map.try_at(*self._current);

                if (not value.has_value()) {
                    CVL_ERROR("Cannot access element of 'cvl::list' before it has been inserted");
                }

                return *value;
            }

            consteval auto operator ->(this const iterator self) -> pointer {
                return std::addressof(*self);
            }

            consteval auto operator ++(this iterator &self) -> iterator & {
                ++self._current;

                return self;
            }

            consteval auto operator ++(this iterator &self, int) -> iterator {
                const auto copy = self;

                ++self;

                return copy;
            }

            consteval auto operator --(this iterator &self) -> iterator & {
                --self._current;

                return self;
            }

            consteval auto operator --(this iterator &self, int) -> iterator {
                const auto copy = self;

                --self;

                return copy;
            }

            friend consteval auto operator ==(const iterator lhs, std::default_sentinel_t) -> bool {
                /* We've reached the end if there is no value for our current index. */

                return not lhs._map.try_at(*lhs._current).has_value();
            }

            /*
                We compare both our members for equality, since
                we need to compare against iterators from any range.
            */
            friend consteval auto operator ==(const iterator lhs, const iterator rhs) -> bool {
                if (lhs._map._tag != rhs._map._tag) {
                    return false;
                }

                return lhs._current == rhs._current;
            }

            friend consteval auto operator <=>(const iterator lhs, const iterator rhs) -> auto {
                /*
                    We compare only our iterators here, since
                    we only need to worry about relational
                    comparisons for iterators of the same range.
                */
                return (lhs._current <=> rhs._current);
            }

            friend consteval auto operator -(const iterator lhs, const iterator rhs) -> difference_type {
                return lhs._current - rhs._current;
            }

            friend consteval auto operator +=(iterator &lhs, const difference_type rhs) -> iterator & {
                lhs._current += rhs;

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
                lhs._current -= rhs;

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

        /*
            A 'cvl::list' is a wrapper over a 'cvl::map'
            with each key being an index into the list.
        */
        cvl::map<std::size_t, T> _map;

        consteval list() = default;

        template<impl::container_compatible_range<T> R>
        consteval list(std::from_range_t, R &&rng) {
            this->append_range(std::forward<R>(rng));
        }

        template<impl::container_compatible_range<T> R>
        requires (not util::unqualified_same_as<R, list>)
        consteval explicit list(R &&rng) : list(std::from_range, std::forward<R>(rng)) {}

        consteval explicit(false) list(const std::initializer_list<T> rng) : list(std::from_range, rng) {}

        consteval auto begin(this const list self) -> iterator {
            return iterator{self._map, std::views::iota(0uz, std::unreachable_sentinel).begin()};
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

        consteval auto _next_index(this const list self) -> std::size_t {
            /* We loop until we find an index value which is not in our map. */

            std::size_t index = 0;

            while (true) {
                if (not self._map.contains(index)) {
                    return index;
                }

                ++index;
            }
        }

        consteval auto push_back(this const list self, const T &elem) -> void {
            const auto next_index = self._next_index();

            self._map.insert(next_index, elem);
        }

        consteval auto try_back(this const list self) -> std::optional<const T &> {
            auto index = self._next_index();

            if (index <= 0) {
                return std::nullopt;
            }

            --index;

            return self._map[index];
        }

        consteval auto try_at(this const list self, const std::size_t index) -> std::optional<const T &> {
            return self._map.try_at(index);
        }

        template<impl::container_compatible_range<T> R>
        consteval auto append_range(this const list self, R &&rng) -> void {
            auto index = self._next_index();

            /* NOTE: 'elem' will have its lifetime extended if initialized by a temporary. */
            for (const T &elem : std::forward<R>(rng)) {
                self._map.insert(index, elem);

                ++index;
            }
        }

        [[nodiscard]]
        consteval auto empty(this const list self) -> bool {
            /* We are empty if the 0-th index has no value. */

            return not self._map.contains(0);
        }

        friend consteval auto operator ==(const list lhs, const list rhs) -> bool {
            if (lhs._map._tag == rhs._map._tag) {
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
