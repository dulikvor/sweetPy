#pragma once

#include <Python.h>
#include "../Types/ObjectPtr.h"


namespace sweetPy {
    namespace Detail {
        
        class ElementValue {
        public:
            ElementValue() = default;
            
            explicit ElementValue(ObjectPtr &&value)
                    : m_value(std::move(value)) {}
            
            ElementValue &operator=(ObjectPtr &&value) {
                m_value = std::move(value);
                return *this;
            }
            
            template<typename Value, typename = enable_if_t <std::is_copy_constructible<Value>::value>>
            Value get() const {
                return Object<Value>::from_python(m_value.get());
            }
            
            template<typename Value, typename = enable_if_t <std::is_copy_constructible<Value>::value>>
            operator Value() {
                return Object<Value>::from_python(m_value.get());
            }
        
        private:
            ObjectPtr m_value;
        };
        
    }
}
