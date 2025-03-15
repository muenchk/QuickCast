#include "DataStorage.h"
#include "Logging.h"
#include "Game.h"
#include "Events.h"
#include "Data.h"
#include "Utility.h"

namespace Storage
{
	/// <summary>
	/// whether processing is enabled
	/// </summary>
	bool processing;
	bool CanProcess() { return processing; }
#define EvalProcessing \
	if (processing == false) \
		return;
#define EvalProcessingBool       \
	if (processing == false) \
		return false;
	

	/// <summary>
	/// Pointer to the singleton of the data class
	/// </summary>
	static Data* data = nullptr;
	void ReadData(SKSE::SerializationInterface* a_intfc);
	void WriteData(SKSE::SerializationInterface* a_intfc);
	void RevertData();

	/// <summary>
	/// Callback executed on saving
	/// saves all global data
	/// </summary>
	/// <param name=""></param>
	void SaveGameCallback(SKSE::SerializationInterface* a_intfc)
	{
		loginfo("[DataStorage] [SaveGameCallback]");
		// save settings in case they have been changed
		WriteData(a_intfc);

		loginfo("[DataStorage] [SaveGameCallback] end");
	}

	/// <summary>
	/// Callback executed on loading
	/// loads all global data
	/// </summary>
	/// <param name=""></param>
	void LoadGameCallback(SKSE::SerializationInterface* a_intfc)
	{
		loginfo("[DataStorage] [LoadGameCallback]");
		ReadData(a_intfc);

		processing = true;
		loginfo("[DataStorage] [LoadGameCallback] end");
	}

	/// <summary>
	/// Callback executed on reverting to older savegame
	/// deletes all active data and disables processing until load event
	/// </summary>
	/// <param name=""></param>
	void RevertGameCallback(SKSE::SerializationInterface* /*a_intfc*/)
	{
		loginfo("[DataStorage] [RevertGameCallback]");
		// save settings in case they have been changed
		if (Settings::_modifiedSettings == Settings::ChangeFlag::kChanged) {
			Settings::Save();
		}
		processing = false;
		RevertData();
		loginfo("[DataStorage] [RevertGameCallback] end");
	}

	void Register()
	{
		Game::SaveLoad::GetSingleton()->RegisterForLoadCallback(0xFF000010, LoadGameCallback);
		loginfo("[DataStorage] [Register] Registered for LoadGameCallback");
		Game::SaveLoad::GetSingleton()->RegisterForRevertCallback(0xFF000020, RevertGameCallback);
		loginfo("[DataStorage] [Register] Registered for RevertGameCallback");
		Game::SaveLoad::GetSingleton()->RegisterForSaveCallback(0xFF000030, SaveGameCallback);
		loginfo("[DataStorage] [Register] Registered for SaveGameCallback");
		data = Data::GetSingleton();
		if (data == nullptr)
			logcritical("[DataStorage] [Register] Cannot access data storage");
	}

	/// <summary>
	/// Reads data from savegame
	/// </summary>
	/// <param name="a_intfc"></param>
	void ReadData(SKSE::SerializationInterface* a_intfc)
	{
		StartProfiling;
		bool preproc = Events::Main::LockProcessing();

		// total number of bytes read
		long size = 0;

		loginfo("[DataStorage] [ReadData] Beginning data load...");

		uint32_t type = 0;
		uint32_t version = 0;
		uint32_t length = 0;

		// for actor info map
		int accounter = 0;
		int acfcounter = 0;
		int acdcounter = 0;

		
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			loginfo("[DataStorage] [ReadData] found record with type {} and length {}", type, length);
			size += length;
			switch (type) {
			case 'ACIF':  // ActorInfo
				size += data->ReadActorInfoMap(a_intfc, length, accounter, acdcounter, acfcounter);
				break;
			case 'DAID':  // Deleted Actor
				size += data->ReadDeletedActors(a_intfc, length);
				break; 
			case 'EDID':  // Dead Actor
				size += Events::Main::ReadDeadActors(a_intfc, length);
				break;
			case 'HOTK':  // Dead Actor
				size += data->ReadHotkeyMap(a_intfc, length);
				break;
			}
		}

		loginfo("[Data] [ReadActorInfoMap] Read {} ActorInfos", accounter);
		loginfo("[Data] [ReadActorInfoMap] Read {} dead, deleted or invalid ActorInfos", acdcounter);
		loginfo("[Data] [ReadActorInfoMap] Failed to read {} ActorInfos", acfcounter);

		loginfo("[DataStorage] [ReadData] Finished loading data");
		if (preproc) {  // if processing was enabled before locking
			Events::Main::UnlockProcessing();
			loginfo("[DataStorage] [ReadData] Enable processing");
		}
		profile(TimeProfiling, "function execution time.");
	}

	/// <summary>
	/// Writes data to savegame
	/// </summary>
	/// <param name="a_intfc"></param>
	void WriteData(SKSE::SerializationInterface* a_intfc)
	{
		StartProfiling;
		bool preproc = Events::Main::LockProcessing();

		// total number of bytes written
		long size = 0;

		loginfo("[DataStorage] [WriteData] Beginning to write data...");
		
		data->CleanActorInfos();
		size += data->SaveActorInfoMap(a_intfc);
		size += data->SaveDeletedActors(a_intfc);
		size += Events::Main::SaveDeadActors(a_intfc);
		size += data->SaveHotkeyMap(a_intfc);

		loginfo("[DataStorage] [WriteData] Finished writing data");

		if (preproc) {  // if processing was enabled before locking
			Events::Main::UnlockProcessing();
			loginfo("[DataStorage] [WriteData] Enable processing");
		}

		profile(TimeProfiling, "function execution time.");
	}

	/// <summary>
	/// Reverts data during game reset
	/// </summary>
	void RevertData()
	{
		bool preproc = Events::Main::LockProcessing();

		loginfo("[DataStorage] [RevertData] Reverting ActorInfo");
		data->DeleteActorInfoMap();

		loginfo("[DataStorage] [RevertData] Finished reverting");

		if (preproc) // if processing was enabled before locking
			Events::Main::UnlockProcessing();
	}
}
