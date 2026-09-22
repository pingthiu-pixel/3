#ifndef ARCHIVER_CORE_H
#define ARCHIVER_CORE_H

#include <string>
#include <vector>
#include <archive.h>
#include <archive_entry.h>
#include <functional>

namespace PArchiver {

struct ArchiveEntry {
    std::string path;
    std::string name;
    size_t size;
    time_t mtime;
    bool isDirectory;
};

struct ArchiveInfo {
    std::string path;
    std::vector<ArchiveEntry> entries;
    size_t totalSize;
    size_t compressedSize;
    int entryCount;
};

class ArchiverCore {
public:
    ArchiverCore();
    ~ArchiverCore();

    // Создание архива
    bool createArchive(const std::string& archivePath, 
                      const std::vector<std::string>& files,
                      const std::string& format = "zip");
    
    // Распаковка архива
    bool extractArchive(const std::string& archivePath, 
                       const std::string& destination);
    
    // Получение информации об архиве
    ArchiveInfo getArchiveInfo(const std::string& archivePath);
    
    // Добавление файлов в существующий архив
    bool addToArchive(const std::string& archivePath, 
                     const std::vector<std::string>& files);
    
    // Удаление файлов из архива
    bool removeFromArchive(const std::string& archivePath, 
                          const std::vector<std::string>& files);
    
    // Тестирование целостности архива
    bool testArchive(const std::string& archivePath);
    
    // Прогресс операции (0.0 - 1.0)
    double getProgress() const { return progress_; }
    
    // Статус операции
    std::string getStatus() const { return status_; }
    
    // Поддерживаемые форматы
    std::vector<std::string> getSupportedFormats() const;

private:
    double progress_;
    std::string status_;
    
    bool writeData(struct archive* arch, const std::string& filepath);
    int copyData(struct archive* ar, struct archive* aw);
};

} // namespace PArchiver

#endif // ARCHIVER_CORE_H
