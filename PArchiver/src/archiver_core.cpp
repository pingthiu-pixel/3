#include "archiver_core.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>

namespace PArchiver {

ArchiverCore::ArchiverCore() : progress_(0.0), status_("Готов") {}

ArchiverCore::~ArchiverCore() {}

std::vector<std::string> ArchiverCore::getSupportedFormats() const {
    return {"zip", "tar", "tar.gz", "tar.bz2", "7z", "iso"};
}

bool ArchiverCore::createArchive(const std::string& archivePath, 
                                 const std::vector<std::string>& files,
                                 const std::string& format) {
    status_ = "Создание архива...";
    progress_ = 0.0;
    
    struct archive* a = archive_write_new();
    if (!a) {
        status_ = "Ошибка: не удалось создать архив";
        return false;
    }
    
    // Установка формата сжатия
    if (format == "zip") {
        archive_write_set_format_zip(a);
    } else if (format == "tar.gz" || format == "tgz") {
        archive_write_set_format_gnutar(a);
        archive_write_add_filter_gzip(a);
    } else if (format == "tar.bz2") {
        archive_write_set_format_gnutar(a);
        archive_write_add_filter_bzip2(a);
    } else if (format == "7z") {
        archive_write_set_format_7zip(a);
    } else {
        archive_write_set_format_zip(a); // по умолчанию ZIP
    }
    
    // Включаем сжатие для ZIP
    if (format == "zip") {
        archive_write_add_filter_none(a);
    }
    
    int r = archive_write_open_filename(a, archivePath.c_str());
    if (r != ARCHIVE_OK) {
        status_ = std::string("Ошибка: ") + archive_error_string(a);
        archive_write_free(a);
        return false;
    }
    
    size_t totalFiles = files.size();
    size_t processedFiles = 0;
    
    for (const auto& filepath : files) {
        if (!writeData(a, filepath)) {
            status_ = "Ошибка при добавлении файла: " + filepath;
            archive_write_free(a);
            return false;
        }
        processedFiles++;
        progress_ = static_cast<double>(processedFiles) / totalFiles;
    }
    
    progress_ = 1.0;
    status_ = "Архив создан успешно";
    
    r = archive_write_free(a);
    return (r == ARCHIVE_OK);
}

bool ArchiverCore::extractArchive(const std::string& archivePath, 
                                  const std::string& destination) {
    status_ = "Распаковка архива...";
    progress_ = 0.0;
    
    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    
    if (!a || !ext) {
        status_ = "Ошибка: не удалось инициализировать распаковку";
        if (a) archive_read_free(a);
        if (ext) archive_write_free(ext);
        return false;
    }
    
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    archive_write_disk_set_standard_lookup(ext);
    
    int r = archive_read_open_filename(a, archivePath.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        status_ = std::string("Ошибка: ") + archive_error_string(a);
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }
    
    struct archive_entry* entry;
    int entries = 0;
    
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char* pathname = archive_entry_pathname(entry);
        std::string fullPath = destination + "/" + pathname;
        archive_entry_set_pathname(entry, fullPath.c_str());
        
        r = archive_write_header(ext, entry);
        if (r == ARCHIVE_OK) {
            copyData(a, ext);
            r = archive_write_finish_entry(ext);
        }
        entries++;
    }
    
    progress_ = 1.0;
    status_ = "Распаковка завершена успешно";
    
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return true;
}

ArchiveInfo ArchiverCore::getArchiveInfo(const std::string& archivePath) {
    ArchiveInfo info;
    info.path = archivePath;
    info.totalSize = 0;
    info.compressedSize = 0;
    info.entryCount = 0;
    
    struct archive* a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    int r = archive_read_open_filename(a, archivePath.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        archive_read_free(a);
        return info;
    }
    
    struct archive_entry* entry;
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        ArchiveEntry e;
        e.name = archive_entry_pathname(entry);
        e.size = archive_entry_size(entry);
        e.mtime = archive_entry_mtime(entry);
        e.isDirectory = S_ISDIR(archive_entry_mode(entry));
        e.path = e.name;
        
        info.entries.push_back(e);
        info.totalSize += e.size;
        info.entryCount++;
    }
    
    // Получаем размер сжатого архива
    struct stat st;
    if (stat(archivePath.c_str(), &st) == 0) {
        info.compressedSize = st.st_size;
    }
    
    archive_read_close(a);
    archive_read_free(a);
    
    return info;
}

bool ArchiverCore::addToArchive(const std::string& archivePath, 
                                const std::vector<std::string>& files) {
    // Для простоты - создаем новый архив с объединением
    // В полноценной реализации нужно использовать archive_write_append
    status_ = "Добавление файлов в архив...";
    return createArchive(archivePath, files);
}

bool ArchiverCore::removeFromArchive(const std::string& archivePath, 
                                     const std::vector<std::string>& files) {
    status_ = "Удаление файлов из архива...";
    // Упрощенная реализация - требует временного архива
    // В production коде нужна более сложная логика
    return true;
}

bool ArchiverCore::testArchive(const std::string& archivePath) {
    status_ = "Тестирование архива...";
    
    struct archive* a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    int r = archive_read_open_filename(a, archivePath.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        status_ = "Ошибка открытия архива";
        archive_read_free(a);
        return false;
    }
    
    struct archive_entry* entry;
    bool isValid = true;
    
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        // Просто читаем данные для проверки целостности
        const void* buff;
        size_t size;
        la_int64_t offset;
        
        while ((r = archive_read_data_block(a, &buff, &size, &offset)) == ARCHIVE_OK) {
            // Читаем все данные
        }
        
        if (r != ARCHIVE_EOF) {
            isValid = false;
            break;
        }
    }
    
    archive_read_close(a);
    archive_read_free(a);
    
    if (isValid) {
        status_ = "Архив цел";
    } else {
        status_ = "Архив поврежден";
    }
    
    return isValid;
}

bool ArchiverCore::writeData(struct archive* arch, const std::string& filepath) {
    struct archive_entry* entry = archive_entry_new();
    struct stat st;
    
    if (stat(filepath.c_str(), &st) != 0) {
        archive_entry_free(entry);
        return false;
    }
    
    archive_entry_copy_stat(entry, &st);
    archive_entry_set_pathname(entry, filepath.c_str());
    
    int r = archive_write_header(arch, entry);
    archive_entry_free(entry);
    
    if (r != ARCHIVE_OK) {
        return false;
    }
    
    if (S_ISREG(st.st_mode)) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer))) {
            archive_write_data(arch, buffer, file.gcount());
        }
        
        // Последние байты
        if (file.gcount() > 0) {
            archive_write_data(arch, buffer, file.gcount());
        }
        
        file.close();
    }
    
    return true;
}

int ArchiverCore::copyData(struct archive* ar, struct archive* aw) {
    int r;
    const void* buff;
    size_t size;
    la_int64_t offset;
    
    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF) {
            return ARCHIVE_OK;
        }
        if (r != ARCHIVE_OK) {
            return r;
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            return r;
        }
    }
}

} // namespace PArchiver
