#include "fsHelper.h"
#include "diag/assert.hpp"
#include "game/BinaryPointers.hpp"
#include "init.h"
#include "logger/Logger.hpp"

namespace FsHelper {
    nn::Result writeFileToPath(void *buf, size_t size, const char *path) {
        nn::fs::FileHandle handle;

        if (isFileExist(path)) {
            Logger::log("Removing Previous File.\n");
            nn::fs::DeleteFile(path); // remove previous file
        }

        if (nn::fs::CreateFile(path, size)) {
            Logger::log("Failed to Create File.\n");
            return 1;
        }

        if (nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Write)) {
            Logger::log("Failed to Open File.\n");
            return 1;
        }

        if (nn::fs::WriteFile(handle, 0, buf, size, nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush))) {
            Logger::log("Failed to Write to File.\n");
            return 1;
        }

        Logger::log("Successfully wrote file to: %s!\n", path);

        nn::fs::CloseFile(handle);

        return 0;
    }

    // make sure to free buffer after usage is done
    void loadFileFromPath(LoadData &loadData) {

        nn::fs::FileHandle handle;

        EXL_ASSERT(FsHelper::isFileExist(loadData.path), "Failed to Find File!\nPath: %s", loadData.path);

        R_ABORT_UNLESS(nn::fs::OpenFile(&handle, loadData.path, nn::fs::OpenMode_Read))

        long size = 0;
        nn::fs::GetFileSize(&size, handle);
        loadData.buffer = BinaryPointers::instance().malloc(ALIGN_UP(size, loadData.alignment));
        loadData.bufSize = ALIGN_UP(size, loadData.alignment);

        EXL_ASSERT(loadData.buffer, "Failed to Allocate Buffer! File Size: %ld", size);

        R_ABORT_UNLESS(nn::fs::ReadFile(handle, 0, loadData.buffer, size))

        nn::fs::CloseFile(handle);
    }


    long getFileSize(const char *path) {
        nn::fs::FileHandle handle;
        long result = -1;

        nn::Result openResult = nn::fs::OpenFile(&handle, path, nn::fs::OpenMode::OpenMode_Read);

        if (openResult.isSuccess()) {
            nn::fs::GetFileSize(&result, handle);
            nn::fs::CloseFile(handle);
        }

        return result;
    }

    bool isFileExist(const char *path) {
        nn::fs::DirectoryEntryType type;
        nn::Result result = nn::fs::GetEntryType(&type, path);

        return result.isSuccess() && type == nn::fs::DirectoryEntryType_File;

    }
}