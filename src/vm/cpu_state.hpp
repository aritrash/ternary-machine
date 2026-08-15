#pragma once

#include "register_file.hpp"

namespace ternary_machine::vm {

	enum class Comparison : std::int8_t {
		Less = -1,
		Equal = 0,
		Greater = 1
	};

	enum class PrivilegeLevel : std::int8_t {
		User = 0,
		Kernel = 1
	};

	enum class TransitionCause : std::int8_t {
		SystemCall = 0,
		Interrupt = 1,
		Exception = 2
	};

	class CPUState final {
		public:
			constexpr CPUState() noexcept = default;

			[[nodiscard]] constexpr const RegisterFile& registers() const noexcept {
				return registers_;
			}

			[[nodiscard]] constexpr RegisterFile& registers() noexcept {
				return registers_;
			}

			[[nodiscard]] constexpr ternary::Word pc() const noexcept {
				return pc_;
			}

			constexpr void set_pc(ternary::Word value) noexcept {
				pc_ = value;
			}

			[[nodiscard]] constexpr ternary::Word sp() const noexcept {
				return sp_;
			}

			constexpr void set_sp(ternary::Word value) noexcept {
				sp_ = value;
			}

			[[nodiscard]] constexpr ternary::Word ksp() const noexcept {
				return ksp_;
			}

			constexpr void set_ksp(ternary::Word value) noexcept {
				ksp_ = value;
			}

			[[nodiscard]] constexpr Comparison status() const noexcept {
				return status_;
			}

			constexpr void set_status(Comparison value) noexcept {
				status_ = value;
			}

			[[nodiscard]] constexpr PrivilegeLevel privilege() const noexcept {
				return privilege_;
			}

			constexpr void set_privilege(PrivilegeLevel value) noexcept {
				privilege_ = value;
			}

			constexpr void reset() noexcept {
				registers_.clear();
				pc_ = ternary::Word::zero();
				sp_ = ternary::Word::zero();
				ksp_ = ternary::Word::zero();
				status_ = Comparison::Equal;
				privilege_ = PrivilegeLevel::Kernel;
			}

		private:
			RegisterFile registers_{};
			ternary::Word pc_{};
			ternary::Word sp_{};
			ternary::Word ksp_{};
			Comparison status_ = Comparison::Equal;
			PrivilegeLevel privilege_ = PrivilegeLevel::Kernel;
	};

}
