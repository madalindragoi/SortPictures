#if !defined (FUNCTIONS_H__)
#define FUNCTIONS_H__

#include "exif.h"
#include "log.hpp"
#include "termcolor.hpp"

#include <windows.h>
#include <fstream>
#include <string>
#include <experimental/filesystem>
#include <iostream>

namespace fs = std::experimental::filesystem;
using namespace termcolor;

    void process_path(std::string& path)
    {
        if (path.at(0) == '\"')
        {
            path.erase(path.begin());
        }

        if (path.back() == '\"')
        {
            path.erase(path.size() - 1);
        }

        if (path.back() != '\\')
        {
            path = path + '\\';
        }

    }

    std::string dateLog()
    {
        time_t curr_time;
        tm* curr_tm;
        char date_string[100];
        char time_string[100];

        time(&curr_time);
        curr_tm = localtime(&curr_time);

        strftime(date_string, 50, "%B %d, %Y", curr_tm);
        strftime(time_string, 50, "%T", curr_tm);

        return std::string() + date_string + " " + time_string + " : ";
    }


    static std::string processDate(std::string& x)
    {
        std::replace(x.begin(), x.end(), ':', '.');
        x = x.substr(0, x.find(' '));

        return x;
    }

   static void createDir(std::string path)
   {
        if (!fs::exists(path))
        {
            fs::create_directory(path);
        }
   }

    static inline bool fileExist(const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    }

    static bool copy_file(const std::string& srce_file, const std::string& dest_file) noexcept
    {
        if (!fileExist(dest_file))
        {
            fs::copy_file(srce_file, dest_file, fs::copy_options::skip_existing);
        }

        if (fileExist(dest_file))
        {
            return true;
        }

        return false;
    }

    static std::string workingdir()
    {
        char buf[256];
        GetCurrentDirectoryA(256, buf);
        return std::string(buf) + '\\';
    }

    std::string result_Path(std::string const& path)
    {
        std::string resultPath = path.substr(0, path.find_last_of("/\\"));
        resultPath = resultPath.substr(0, resultPath.find_last_of("/\\")) + "\\SortPicturesResult\\";

        return resultPath;
    }

    static bool sortPictures(std::string const& path)
    {
        try
        {
            unsigned int corupted = 0u;
            unsigned int sorted = 0u;
            unsigned int unsorted = 0u;
            unsigned int errorCopy = 0u;

            bool corupt = false;
            bool unsort = false;



            std::string resultPath = result_Path(path);
            createDir(resultPath);

            std::string logpath;

            if (!fs::exists(resultPath))
            {
                logpath = workingdir() + "sortPictures.log";
            }
            else if (resultPath.back() == '\\')
            {
                logpath = resultPath + "sortPictures.log";
            }
            else
            {
                logpath = resultPath + "\\sortPictures.log";
            }

            xtd::log log(logpath);
                
            if (!fs::exists(path))
            {
                log.write(dateLog(), "ERROR: ","Path Not Found\n\n");
                std::cout << red << dateLog() << "ERROR: " << "Path Not Found\n";
                MessageBox(0, "Path not found", "Sort Pictures", MB_ICONERROR);
                return false;
            }

            for (const auto& entry : fs::recursive_directory_iterator(path))
            {
                std::string entrypath = entry.path().string();
                std::replace(entrypath.begin(), entrypath.end(), '\\', '/');
                std::string sortedPath = "";
                const char* paths = entrypath.c_str();

                FILE* fp = fopen(paths, "rb");

                if (entry.path().extension() == ".jpg" || entry.path().extension() == ".jpeg" || entry.path().extension() == ".JPG" || entry.path().extension() == ".JPEG")
                {
                    fseek(fp, 0, SEEK_END);

                    unsigned long fsize = ftell(fp);
                    rewind(fp);
                    unsigned char* buf = new unsigned char[fsize];

                    if (fread(buf, 1, fsize, fp) != fsize)
                    {
                        log.write(dateLog(), "ERROR: ","Cannot read the ", entrypath.c_str(), " file\n\n");
                        std::cout << red << "ERROR: " << "Cannot read the " << entrypath.c_str() << "file\n";
                        delete[] buf;                    
                    }

                    fclose(fp);

                    easyexif::EXIFInfo result;
                    int code = result.parseFrom(buf, fsize);
                    delete[] buf;

                    if (0 == code) 
                    {
                        sortedPath = resultPath + "Sorted";
                        ++sorted;
                    }
                    else if (1983 == code)
                    {
                        sortedPath = resultPath + "Unsorted";
                        ++unsorted;
                        unsort = true;
                    }
                    else
                    {
                        sortedPath = resultPath + "Corupted";
                        ++corupted;
                        corupt = true;
                    }

                    result.DateTimeOriginal = processDate(result.DateTimeOriginal);

                    createDir(sortedPath);

                    std::string dirpath = sortedPath + "\\" + result.DateTimeOriginal;
                    std::string fileName = fs::path(entry).filename().string();
                    process_path(dirpath);
                    std::string destFile = dirpath + fileName;

                    createDir(dirpath);

                    if (!copy_file(entrypath, destFile))
                    {
                        ++errorCopy;
                    }
                
                    if (unsort)
                    {
                        log.write(dateLog(), "INFO:", " Copied ", entrypath.c_str(), " to ", destFile, "\n\n");
                        std::cout << yellow << dateLog() << "INFO:" << " Copied " << entrypath.c_str() << " to " << destFile << "\n";
                        unsort = false;
                    }
                    else if (corupt)
                    {
                        log.write(dateLog(), "WARNING (Corupted file):", " Copied ", entrypath.c_str(), " to ", destFile, "\n\n");
                        std::cout << red << dateLog() << "WARNING (Corupted file):" << " Copied " << entrypath.c_str() << " to " << destFile << "\n";
                        corupt = false;
                    }

                    else
                    {
                        log.write(dateLog(), "INFO:", " Copied ", entrypath.c_str(), " to ", destFile, "\n\n");
                        std::cout << green << dateLog() << "INFO:" << " Copied " << entrypath.c_str() << " to " << destFile << "\n";
                    }
                }
            }
            
            unsigned int totalPictures = sorted + unsorted + corupted;

            log.write("Summary:\n");
            log.write("\n\nTotal pictures: ", totalPictures, " Sorted: ", sorted, " Unsorted: ", unsorted, " Corupted: ", corupted, " Not copied: ", errorCopy);

            std::cout << white << "\n\nTotal pictures: " << totalPictures << green << "\tSorted: " << sorted
                << yellow << "\tUnsorted: " << unsorted << red << "\tCorupted: " << corupted << "\t Not copied: " << errorCopy << white;
        }
        catch (...)
        {
            std::cout << white;
            MessageBox(0, "Path not found", "Sort Pictures", MB_ICONERROR);
        }          

        return true;
    }

#endif

