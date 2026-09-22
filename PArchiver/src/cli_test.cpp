#include <iostream>
#include "archiver_core.h"

int main(int argc, char* argv[]) {
    std::cout << "PArchiver CLI - Тестирование ядра архиватора" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    PArchiver::ArchiverCore archiver;
    
    // Показываем поддерживаемые форматы
    std::cout << "\nПоддерживаемые форматы:" << std::endl;
    auto formats = archiver.getSupportedFormats();
    for (const auto& fmt : formats) {
        std::cout << "  - " << fmt << std::endl;
    }
    
    // Тест создания архива
    if (argc > 1) {
        std::vector<std::string> files;
        for (int i = 1; i < argc; ++i) {
            files.push_back(argv[i]);
        }
        
        std::cout << "\nСоздание архива test_archive.zip..." << std::endl;
        if (archiver.createArchive("test_archive.zip", files)) {
            std::cout << "✓ Архив успешно создан!" << std::endl;
            
            // Получаем информацию об архиве
            std::cout << "\nИнформация об архиве:" << std::endl;
            auto info = archiver.getArchiveInfo("test_archive.zip");
            std::cout << "  Файлов в архиве: " << info.entryCount << std::endl;
            std::cout << "  Общий размер: " << info.totalSize << " байт" << std::endl;
            std::cout << "  Размер архива: " << info.compressedSize << " байт" << std::endl;
            
            // Тестируем архив
            std::cout << "\nТестирование целостности..." << std::endl;
            if (archiver.testArchive("test_archive.zip")) {
                std::cout << "✓ Архив цел!" << std::endl;
            } else {
                std::cout << "✗ Архив поврежден!" << std::endl;
            }
            
            // Распаковка
            std::cout << "\nРаспаковка в директорию 'extracted'..." << std::endl;
            if (archiver.extractArchive("test_archive.zip", "extracted")) {
                std::cout << "✓ Распаковка успешна!" << std::endl;
            } else {
                std::cout << "✗ Ошибка распаковки!" << std::endl;
            }
        } else {
            std::cout << "✗ Ошибка создания архива!" << std::endl;
            std::cout << "Статус: " << archiver.getStatus() << std::endl;
        }
    } else {
        std::cout << "\nИспользование: " << argv[0] << " <файл1> [файл2] ..." << std::endl;
        std::cout << "\nПример:" << std::endl;
        std::cout << "  ./parchiver_cli file1.txt file2.txt" << std::endl;
    }
    
    std::cout << "\nСтатус: " << archiver.getStatus() << std::endl;
    std::cout << "Прогресс: " << (archiver.getProgress() * 100) << "%" << std::endl;
    
    return 0;
}
