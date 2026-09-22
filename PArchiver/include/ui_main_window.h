#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "archiver_core.h"

namespace PArchiver {

class MainWindow : public sf::RenderWindow {
public:
    MainWindow();
    ~MainWindow();

    void run();

private:
    // Компоненты интерфейса
    sf::RectangleShape header_;
    sf::RectangleShape contentArea_;
    sf::RectangleShape sidebar_;
    sf::RectangleShape statusBar_;
    
    // Кнопки
    std::vector<std::pair<sf::RectangleShape, std::string>> buttons_;
    
    // Список файлов
    sf::RectangleShape fileListBackground_;
    std::vector<std::pair<sf::RectangleShape, std::string>> fileListItems_;
    
    // Текстовые элементы
    sf::Font font_;
    std::vector<sf::Text> texts_;
    
    // Состояние приложения
    enum class State {
        MAIN,
        ARCHIVE_OPEN,
        SETTINGS,
        ABOUT
    };
    State currentState_;
    
    ArchiverCore archiver_;
    ArchiveInfo currentArchive_;
    bool hasOpenArchive_;
    
    // Методы отрисовки
    void initUI();
    void drawHeader();
    void drawSidebar();
    void drawContentArea();
    void drawStatusBar();
    void drawFileList();
    void drawButtons();
    
    // Обработка событий
    void handleEvents();
    void handleMouseClick(sf::Vector2i mousePos);
    void checkButtonClick(sf::Vector2f mousePos);
    void checkFileListClick(sf::Vector2f mousePos);
    
    // Действия
    void createNewArchive();
    void openArchive();
    void extractArchive();
    void addToArchive();
    void showAbout();
    void refreshFileList();
    
    // Утилиты
    sf::Color hexToColor(const std::string& hex);
    void centerText(sf::Text& text, const sf::RectangleShape& shape);
};

} // namespace PArchiver

#endif // UI_MAIN_WINDOW_H
