#pragma once

#include <memory>
#include <stdexcept>

namespace rawrbox {
	template <class T>
	class Reference {
		std::shared_ptr<T*> ptr;

	public:
		[[nodiscard]] bool valid() const {
			return ptr.get() != nullptr && *ptr.get() != nullptr;
		}

		void reset() {
			ptr.reset();
		}

		template <class RetType = T>
		RetType& get() const {
			auto obj = *ptr.get();
			if (obj == nullptr) throw std::runtime_error("object is nullptr");
			return *dynamic_cast<RetType*>(obj);
		}

		template <class RetType = T>
		[[nodiscard]] bool checkType() const {
			auto obj = *ptr.get();
			if (obj == nullptr) throw std::runtime_error("object is nullptr");
			return dynamic_cast<RetType*>(obj) != nullptr;
		}

		bool operator==(const Reference<T>& other) const {
			return &this->get() == &other.get();
		}

		bool operator!=(const Reference<T>& other) const {
			return !operator==(other);
		}

		Reference() = default;
		Reference(const std::shared_ptr<T*>& ptr_) : ptr(ptr_) {}
	};

	template <class T>
	class ReferenceContainer {
		std::shared_ptr<T*> ref;

	public:
		Reference<T> getReference() {
			return Reference<T>(ref);
		}

		ReferenceContainer(const ReferenceContainer&) = delete;
		ReferenceContainer(ReferenceContainer&&) = delete;
		ReferenceContainer& operator=(const ReferenceContainer&) = delete;
		ReferenceContainer& operator=(ReferenceContainer&&) = delete;
		ReferenceContainer(T* thisPtr) : ref(std::make_shared<T*>()) {
			*ref = thisPtr;
		}

		~ReferenceContainer() {
			*ref = nullptr;
		}
	};
} // namespace rawrbox
