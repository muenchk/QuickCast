#pragma once
namespace Hooks
{

	class OnFrameHook
	{
	public:
		static void Install()
		{
			REL::Relocation<uintptr_t> target{ REL::VariantID(35564, 36563, 0x5baa00), REL::VariantOffset(0x24, 0x24, 0x27) };
			auto& trampoline = SKSE::GetTrampoline();

			switch (REL::Module::GetRuntime()) {
			case REL::Module::Runtime::AE:
				_OnFrame = trampoline.write_call<5>(target.address(), OnFrame);
				break;
			case REL::Module::Runtime::SE:
				_OnFrame = trampoline.write_call<5>(target.address(), OnFrame);
				break;
			case REL::Module::Runtime::VR:
				_OnFrame = trampoline.write_call<6>(target.address(), OnFrame);
				break;
			}

			logger::info("Hooked Main::FrameUpdate");
		}

	private:
		static uint64_t OnFrame(void* unk);
		static inline REL::Relocation<decltype(OnFrame)> _OnFrame;
	};

	/// <summary>
	/// executed when confirming fast travel message box on world map
	/// </summary>
	class FastTravelConfirmHook
	{
	public:
		static void InstallHook() {
			REL::Relocation<uintptr_t> target{ REL::VariantID(52236, 53127, 0x91a4e0), REL::VariantOffset(0x31, 0x31, 0x31) };
			auto& trampoline = SKSE::GetTrampoline();

			_FastTravelConfirm = trampoline.write_call<5>(target.address(), FastTravelConfirm);
		}

	private:
		static bool FastTravelConfirm(uint64_t self, uint64_t menu);
		static inline REL::Relocation<decltype(FastTravelConfirm)> _FastTravelConfirm;
	};

	class Papyrus_FastTravelHook
	{
	private:
		static inline uint64_t remainder_1;
		static inline uint64_t remainder_2;

	public:
		static void InstallHook()
		{
			REL::Relocation<uintptr_t> targetbegin{ REL::VariantID(54824, 55457, 0), REL::VariantOffset(0x78, 0x78, 0) };
			REL::Relocation<uintptr_t> targetend{ REL::VariantID(54824, 55457, 0), REL::VariantOffset(0xD7, 0xD7, 0) };
			auto& trampoline = SKSE::GetTrampoline();

			
			struct Patch : Xbyak::CodeGenerator
			{
				Patch(uintptr_t a_remainder, uintptr_t a_fastTravelBegin)
				{
					Xbyak::Label fdec;

					mov(ptr[rsp + 0x50], rcx);
					//mov(rdi, rdx);
					//mov(rsi, rcx);
					call(ptr[rip + fdec]);

					mov(rax, qword[a_remainder]);
					jmp(rax);

					L(fdec);
					dq(a_fastTravelBegin);
				}
			};

			Patch patch{ (uintptr_t)(&remainder_1), reinterpret_cast<uintptr_t>(FastTravelBegin) };
			patch.ready();

			remainder_1 = trampoline.write_branch<5>(targetbegin.address(), trampoline.allocate(patch));
			remainder_1 = targetbegin.address() + 0x5;
		}

	private:
		static void FastTravelBegin();

		static void FastTravelEnd();

	};

	class FavoritesHandlerEx : public RE::FavoritesHandler
	{
	public:
		bool ProcessButton_Hook(RE::ButtonEvent* a_event);  // 05

		static void InstallHook();

		using ProcessButton_t = decltype(&RE::FavoritesHandler::ProcessButton);
		static inline REL::Relocation<ProcessButton_t> _ProcessButton;
	};

	class EquipObjectOverRide
	{
	public:
		static void thunk(RE::ActorEquipManager* a_self, RE::Actor* a_actor, RE::TESBoundObject* a_object, std::uint64_t a_unk);
		static inline REL::Relocation<decltype(thunk)> func;
		static void Install();
	};

	class UIHooks
	{
	public:
		static void Hook()
		{
			const REL::Relocation<uintptr_t> inputHook{ REL::VariantID(67315, 68617, 0xC519E0) };           // C150B0, C3B360, C519E0

			auto& trampoline = SKSE::GetTrampoline();

			SKSE::AllocTrampoline(14);
			_InputFunc = trampoline.write_call<5>(inputHook.address() + REL::VariantOffset(0x7B, 0x7B, 0x81).offset(), InputFunc);

		}
		static void InputFunc(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_events);

		static inline REL::Relocation<decltype(InputFunc)> _InputFunc;
	};

	void InstallHooks();
}


