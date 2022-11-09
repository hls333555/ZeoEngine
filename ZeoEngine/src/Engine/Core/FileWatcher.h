#pragma once

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
		FileWatcher(std::vector<std::filesystem::path> directoriesToWatch, std::chrono::duration<I32, std::milli> interval)
			: m_DirectoriesToWatch(std::move(directoriesToWatch))
			, m_Interval(interval)
		{
			for (auto& directoryToWatch : m_DirectoriesToWatch)
			{
				// Directory to watch must use absolute path!
				directoryToWatch = std::filesystem::canonical(directoryToWatch);
				for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryToWatch))
				{
					if (entry.is_directory()) continue;

					m_WatchedFiles[entry.path()] = std::filesystem::last_write_time(entry);
				}
			}

			ZE_CORE_TRACE("File watcher initialized");

			Start();
		}

		~FileWatcher()
		{
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				m_bIsRunning = false;
			}
			m_CV.notify_one();
			m_FileWatcherThread->join();
		}
		
		void Start()
		{
			m_FileWatcherThread = CreateScope<std::thread>(&FileWatcher::Execute, this);
		}

	private:
		// https://stackoverflow.com/a/52611091/13756224
		// Returns false if m_bIsRunning = false
		template<class Duration>
		bool WaitFor(Duration duration)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			return !m_CV.wait_for(lock, duration, [this]() { return !m_bIsRunning; });
		}

		/** Monitor "m_DirectoryToWatch" for changes and in case of a change execute the delegate. */
		void Execute()
		{
			while (WaitFor(m_Interval))
			{
				auto it = m_WatchedFiles.begin();
				while (it != m_WatchedFiles.end())
				{
					// File removal
					if (!std::filesystem::exists(it->first))
					{
						m_OnFileRemovedDel.publish(it->first.string());
						it = m_WatchedFiles.erase(it);
					}
					else
					{
						++it;
					}
				}

				for (const auto& directoryToWatch : m_DirectoriesToWatch)
				{
					if (std::filesystem::exists(directoryToWatch))
					{
						// Check if a file was created or modified
						for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryToWatch))
						{
							if (entry.is_directory()) continue;

							auto currentFileLastWriteTime = std::filesystem::last_write_time(entry);

							const auto& filePath = entry.path();
							// File creation
							if (m_WatchedFiles.find(filePath) == m_WatchedFiles.end())
							{
								m_WatchedFiles[filePath] = currentFileLastWriteTime;
								m_OnFileAddedDel.publish(filePath.string());
							}
							// File modification
							else
							{
								if (m_WatchedFiles[filePath] != currentFileLastWriteTime)
								{
									m_WatchedFiles[filePath] = currentFileLastWriteTime;
									m_OnFileModifiedDel.publish(filePath.string());
								}
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
		std::vector<std::filesystem::path> m_DirectoriesToWatch;
		/** Time interval at which we check the base folder for changes */
		std::chrono::duration<I32, std::milli> m_Interval;

		/** A record of files from the base directory to their last modification time */
		std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> m_WatchedFiles;

		mutable std::mutex m_Mutex;
		mutable std::condition_variable m_CV;
		bool m_bIsRunning = true;
		Scope<std::thread> m_FileWatcherThread;

		entt::sigh<void(const std::string&)> m_OnFileAddedDel, m_OnFileModifiedDel, m_OnFileRemovedDel;
	};

}
