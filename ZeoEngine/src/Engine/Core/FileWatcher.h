#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <thread>
#include <entt.hpp>

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class FileWatcher
	{
		friend void Execute(FileWatcher& fileWatcher);

	public:
		FileWatcher(const std::string& directoryToWatch, std::chrono::duration<int32_t, std::milli> delay)
			: m_DirectoryToWatch(directoryToWatch), m_Delay(delay)
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryToWatch))
			{
				if (entry.is_directory()) continue;

				m_WatchedFiles[entry.path().string()] = std::filesystem::last_write_time(entry);
			}

			ZE_CORE_TRACE("File watcher initialized");

			Start();
		}
		
		void Start()
		{
			m_bIsRunning = true;
			m_FileWatcherThread = CreateScope<std::thread>(Execute, std::ref(*this));
			m_FileWatcherThread->detach();
		}

		void Stop()
		{
			m_bIsRunning = true;
		}

	public:
		entt::sink<void(const std::string&)> m_OnFileAdded{ m_OnFileAddedDel };
		entt::sink<void(const std::string&)> m_OnFileModified{ m_OnFileModifiedDel };
		entt::sink<void(const std::string&)> m_OnFileRemoved{ m_OnFileRemovedDel };

	private:
		std::string m_DirectoryToWatch;
		/** Time interval at which we check the base folder for changes */
		std::chrono::duration<int32_t, std::milli> m_Delay;

		/** A record of files from the base directory and their last modification time */
		std::unordered_map<std::string, std::filesystem::file_time_type> m_WatchedFiles;

		bool m_bIsRunning = true;
		Scope<std::thread> m_FileWatcherThread;

		entt::sigh<void(const std::string&)> m_OnFileAddedDel, m_OnFileModifiedDel, m_OnFileRemovedDel;
	};

	/** Monitor "m_DirectoryToWatch" for changes and in case of a change execute the user supplied "action" function. */
	static void Execute(FileWatcher& fileWatcher)
	{
		while (fileWatcher.m_bIsRunning)
		{
			// Wait for "m_Delay" milliseconds
			std::this_thread::sleep_for(fileWatcher.m_Delay);

			auto it = fileWatcher.m_WatchedFiles.begin();
			while (it != fileWatcher.m_WatchedFiles.end())
			{
				// File removal
				if (!std::filesystem::exists(it->first))
				{
					fileWatcher.m_OnFileRemovedDel.publish(it->first);
					it = fileWatcher.m_WatchedFiles.erase(it);
				}
				else
				{
					++it;
				}
			}

			// Check if a file was created or modified
			for (const auto& entry : std::filesystem::recursive_directory_iterator(fileWatcher.m_DirectoryToWatch))
			{
				if (entry.is_directory()) continue;

				auto currentFileLastWriteTime = std::filesystem::last_write_time(entry);

				std::string filePath = entry.path().string();
				// File creation
				if (fileWatcher.m_WatchedFiles.find(filePath) == fileWatcher.m_WatchedFiles.end())
				{
					fileWatcher.m_WatchedFiles[filePath] = currentFileLastWriteTime;
					fileWatcher.m_OnFileAddedDel.publish(filePath);
				}
				// File modification
				else
				{
					if (fileWatcher.m_WatchedFiles[filePath] != currentFileLastWriteTime)
					{
						fileWatcher.m_WatchedFiles[filePath] = currentFileLastWriteTime;
						fileWatcher.m_OnFileModifiedDel.publish(filePath);
					}
				}
			}
		}
	}

}
