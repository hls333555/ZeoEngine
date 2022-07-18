#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <thread>
#include <entt.hpp>

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	/** A file watcher running on a separate thread. */
	class FileWatcher
	{
	public:
		FileWatcher(const std::string& directoryToWatch, std::chrono::duration<I32, std::milli> delay)
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

		~FileWatcher()
		{
			Stop();
		}
		
		void Start()
		{
			m_bIsRunning = true;
			m_FileWatcherThread = CreateScope<std::thread>(&FileWatcher::Execute, this);
			m_FileWatcherThread->detach();
		}

		void Stop()
		{
			m_bIsRunning = false;
		}

	private:
		/** Monitor "m_DirectoryToWatch" for changes and in case of a change execute the delegate. */
		void Execute()
		{
			while (m_bIsRunning)
			{
				// Wait for "m_Delay" milliseconds
				std::this_thread::sleep_for(m_Delay);

				auto it = m_WatchedFiles.begin();
				while (it != m_WatchedFiles.end())
				{
					// File removal
					if (!std::filesystem::exists(it->first))
					{
						m_OnFileRemovedDel.publish(it->first);
						it = m_WatchedFiles.erase(it);
					}
					else
					{
						++it;
					}
				}

				std::filesystem::path watchedPath(m_DirectoryToWatch);
				if (std::filesystem::exists(watchedPath))
				{
					// Check if a file was created or modified
					for (const auto& entry : std::filesystem::recursive_directory_iterator(watchedPath))
					{
						if (entry.is_directory()) continue;

						auto currentFileLastWriteTime = std::filesystem::last_write_time(entry);

						std::string filePath = entry.path().string();
						// File creation
						if (m_WatchedFiles.find(filePath) == m_WatchedFiles.end())
						{
							m_WatchedFiles[filePath] = currentFileLastWriteTime;
							m_OnFileAddedDel.publish(filePath);
						}
						// File modification
						else
						{
							if (m_WatchedFiles[filePath] != currentFileLastWriteTime)
							{
								m_WatchedFiles[filePath] = currentFileLastWriteTime;
								m_OnFileModifiedDel.publish(filePath);
							}
						}
					}
				}
			}
		}

	public:
		entt::sink<entt::sigh<void(const std::string&)>> m_OnFileAdded{ m_OnFileAddedDel };
		entt::sink<entt::sigh<void(const std::string&)>> m_OnFileModified{ m_OnFileModifiedDel };
		entt::sink<entt::sigh<void(const std::string&)>> m_OnFileRemoved{ m_OnFileRemovedDel };

	private:
		std::string m_DirectoryToWatch;
		/** Time interval at which we check the base folder for changes */
		std::chrono::duration<I32, std::milli> m_Delay;

		/** A record of files from the base directory and their last modification time */
		std::unordered_map<std::string, std::filesystem::file_time_type> m_WatchedFiles;

		bool m_bIsRunning = true;
		Scope<std::thread> m_FileWatcherThread;

		entt::sigh<void(const std::string&)> m_OnFileAddedDel, m_OnFileModifiedDel, m_OnFileRemovedDel;
	};

}
