#pragma once

#include <cvl/common.hpp>
#include <cvl/util.hpp>

namespace cvl {

    namespace impl {

        template<util::tag, auto Key, typename Value>
        constexpr inline cvl::delayed_init<Value> map_value_holder;

    }

    template<util::constant_reflectable Key, util::constant_reflectable Value>
    requires (not std::is_reference_v<Key> and not std::is_reference_v<Value>)
    struct map {
        util::tag _tag;

        consteval auto _value_holder(this const map self, const Key &key) -> cvl::delayed_init<Value> {
            return extract<cvl::delayed_init<Value>>(
                self._tag.substitute(^^impl::map_value_holder, {
                    std::meta::reflect_constant(key),
                    ^^Value
                })
            );
        }

        consteval auto contains(this const map self, const Key &key) -> bool {
            return self._value_holder(key).has_value();
        }

        consteval auto try_insert(this const map self, const Key &key, const Value &value) -> bool {
            const auto holder = self._value_holder(key);

            if (holder.has_value()) {
                return false;
            }

            holder = value;

            return true;
        }

        consteval auto insert(this const map self, const Key &key, const Value &value) -> void {
            if (not self.try_insert(key, value)) {
                CVL_ERROR("Cannot insert value for key which already has a value inserted");
            }
        }

        consteval auto try_at(this const map self, const Key &key) -> std::optional<const Value &> {
            return self._value_holder(key).optional();
        }

        consteval auto operator [](this const map self, const Key &key) -> const Value & {
            const auto value = self.try_at(key);

            if (not value.has_value()) {
                CVL_ERROR("Cannot access value for key which has no value inserted");
            }

            return *value;
        }
    };

}
