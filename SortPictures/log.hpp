#if !defined (LOG_H__)
#define LOG_H__

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <fstream>
#include <time.h>
#include <sstream>

namespace xtd
{
	class log
	{
		using log_type = std::ofstream;
		using log_path = std::string;

	private:
		log_type log_;
		log_path logpath_;
	public:
		log(log_path logpath) :
			logpath_(logpath)
		{
			log_.open(logpath_, std::ifstream::binary);
			
			if (!log_.is_open())
			{
				throw "Cannot open log file";
			}
		}

		log() = delete;

		~log() = default;

		template<typename T>
		void write(const T& first)
		{
			log_ << first;
		}

		template<typename T, typename... Args>
		void write(const T& first, const Args&... args)
		{
			log_ << first;
			write(args...);
		}
	};
}

#endif