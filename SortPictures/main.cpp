#include "functions.hpp"

int main(int argc, char* argv[]) 
{

    std::string path;
    std::string msg;

    if (argc > 1)
    {
        path = workingdir();
    }
    else
    {
        std::cout << white << "Insert path: ";
        std::cin >> path;
    }

    process_path(path);

    std::string logpath;
    try
    {
        if (!fs::exists(path))
        {
            logpath = workingdir() + "\\sortPictures.log";
        }
        else if (path.back() == '\\')
        {
            logpath = result_Path(path) + "sortPictures.log";
        }
        else
        {
            logpath = result_Path(path) + "\\sortPictures.log";
        }

        if (sortPictures(path))
        {
            msg = "Succes, log created: " + logpath;
            MessageBox(0, msg.c_str(), "Sort Pictures", MB_ICONINFORMATION);
        }
        else
        {
            msg = "Failed, log created: " + logpath;
            std::cout << white;
            MessageBox(0, msg.c_str(), "Sort Pictures", MB_ICONERROR);
        }
    }
    catch (...)
    {
        std::cout << white;
        return -1;
    }

    return 0;
}
