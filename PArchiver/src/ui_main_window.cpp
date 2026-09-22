#include "ui_main_window.h"
#include <iostream>
#include <fstream>

namespace PArchiver {

// Современная цветовая палитра
static const sf::Color COLOR_PRIMARY = sf::Color(76, 175, 80);      // Material Green 500
static const sf::Color COLOR_PRIMARY_DARK = sf::Color(56, 142, 60);  // Material Green 700
static const sf::Color COLOR_ACCENT = sf::Color(255, 193, 7);        // Amber 500
static const sf::Color COLOR_BACKGROUND = sf::Color(245, 245, 245);  // Light gray
static const sf::Color COLOR_SURFACE = sf::Color(255, 255, 255);     // White
static const sf::Color COLOR_TEXT_PRIMARY = sf::Color(33, 33, 33);   // Dark gray
static const sf::Color COLOR_TEXT_SECONDARY = sf::Color(117, 117, 117);
static const sf::Color COLOR_ERROR = sf::Color(244, 67, 54);         // Red 500

MainWindow::MainWindow() 
    : sf::RenderWindow(sf::VideoMode(1200, 800), "PArchiver - Современный архиватор"),
      currentState_(State::MAIN),
      hasOpenArchive_(false) {
    
    setFramerateLimit(60);
    
    // Загрузка шрифта из файла
    if (!font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cout << "Предупреждение: не удалось загрузить шрифт DejaVuSans" << std::endl;
    }
    
    initUI();
}

MainWindow::~MainWindow() {}

void MainWindow::initUI() {
    // Инициализация компонентов интерфейса будет здесь
    refreshFileList();
}

void MainWindow::run() {
    while (isOpen()) {
        handleEvents();
        
        clear(COLOR_BACKGROUND);
        
        drawHeader();
        drawSidebar();
        drawContentArea();
        drawStatusBar();
        drawFileList();
        drawButtons();
        
        display();
    }
}

void MainWindow::drawHeader() {
    // Верхняя панель с градиентом
    header_.setSize(sf::Vector2f(1200, 60));
    header_.setPosition(0, 0);
    header_.setFillColor(COLOR_PRIMARY);
    draw(header_);
    
    // Заголовок приложения
    sf::Text title("PArchiver", font_, 24);
    title.setFillColor(sf::Color::White);
    title.setPosition(20, 15);
    draw(title);
    
    // Подзаголовок
    sf::Text subtitle("Современный архиватор файлов", font_, 12);
    subtitle.setFillColor(sf::Color(200, 255, 200));
    subtitle.setPosition(20, 40);
    draw(subtitle);
}

void MainWindow::drawSidebar() {
    // Боковая панель
    sidebar_.setSize(sf::Vector2f(250, 740));
    sidebar_.setPosition(0, 60);
    sidebar_.setFillColor(COLOR_SURFACE);
    draw(sidebar_);
    
    // Кнопки боковой панели
    float btnWidth = 230;
    float btnHeight = 45;
    float startY = 80;
    float gap = 10;
    
    std::vector<std::pair<std::string, std::function<void()>>> sidebarButtons = {
        {"📁 Новый архив", [this]() { createNewArchive(); }},
        {"📂 Открыть архив", [this]() { openArchive(); }},
        {"📤 Распаковать", [this]() { extractArchive(); }},
        {"➕ Добавить файлы", [this]() { addToArchive(); }},
        {"ℹ️ О программе", [this]() { showAbout(); }}
    };
    
    buttons_.clear();
    for (size_t i = 0; i < sidebarButtons.size(); ++i) {
        sf::RectangleShape btn(sf::Vector2f(btnWidth, btnHeight));
        btn.setPosition(10, startY + i * (btnHeight + gap));
        btn.setFillColor(COLOR_PRIMARY);
        btn.setOutlineThickness(1);
        btn.setOutlineColor(COLOR_PRIMARY_DARK);
        
        buttons_.push_back({btn, sidebarButtons[i].first});
        
        sf::Text text(sidebarButtons[i].first, font_, 14);
        text.setFillColor(sf::Color::White);
        text.setPosition(25, startY + i * (btnHeight + gap) + 12);
        texts_.push_back(text);
    }
}

void MainWindow::drawContentArea() {
    // Основная область контента
    contentArea_.setSize(sf::Vector2f(940, 680));
    contentArea_.setPosition(260, 60);
    contentArea_.setFillColor(COLOR_SURFACE);
    contentArea_.setOutlineThickness(1);
    contentArea_.setOutlineColor(sf::Color(200, 200, 200));
    draw(contentArea_);
    
    // Заголовок области контента
    std::string contentTitle = hasOpenArchive_ ? 
                               "Архив: " + currentArchive_.path : 
                               "Добро пожаловать в PArchiver";
    sf::Text title(contentTitle, font_, 18);
    title.setFillColor(COLOR_TEXT_PRIMARY);
    title.setPosition(280, 75);
    draw(title);
    
    // Информация об архиве (если открыт)
    if (hasOpenArchive_) {
        std::string info = "Файлов: " + std::to_string(currentArchive_.entryCount) +
                          " | Размер: " + std::to_string(currentArchive_.totalSize / 1024) + " KB";
        sf::Text infoText(info, font_, 14);
        infoText.setFillColor(COLOR_TEXT_SECONDARY);
        infoText.setPosition(280, 100);
        draw(infoText);
    } else {
        sf::Text hint("Выберите действие в меню слева или перетащите файлы сюда", font_, 14);
        hint.setFillColor(COLOR_TEXT_SECONDARY);
        hint.setPosition(280, 300);
        draw(hint);
    }
}

void MainWindow::drawStatusBar() {
    // Строка состояния внизу
    statusBar_.setSize(sf::Vector2f(1200, 30));
    statusBar_.setPosition(0, 770);
    statusBar_.setFillColor(COLOR_PRIMARY_DARK);
    draw(statusBar_);
    
    std::string status = archiver_.getStatus();
    sf::Text statusText(status, font_, 12);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10, 775);
    draw(statusText);
}

void MainWindow::drawFileList() {
    // Список файлов в архиве
    if (!hasOpenArchive_ || currentArchive_.entries.empty()) {
        return;
    }
    
    fileListBackground_.setSize(sf::Vector2f(920, 550));
    fileListBackground_.setPosition(270, 130);
    fileListBackground_.setFillColor(sf::Color(250, 250, 250));
    fileListBackground_.setOutlineThickness(1);
    fileListBackground_.setOutlineColor(sf::Color(220, 220, 220));
    draw(fileListBackground_);
    
    // Заголовки таблицы
    std::vector<std::string> headers = {"Имя", "Размер", "Дата изменения", "Тип"};
    float xPos[] = {280, 550, 700, 850};
    
    for (int i = 0; i < 4; ++i) {
        sf::Text header(headers[i], font_, 12);
        header.setFillColor(COLOR_TEXT_PRIMARY);
        header.setPosition(xPos[i], 140);
        header.setStyle(sf::Text::Bold);
        draw(header);
    }
    
    // Элементы списка
    fileListItems_.clear();
    int y = 165;
    for (const auto& entry : currentArchive_.entries) {
        sf::RectangleShape item(sf::Vector2f(910, 25));
        item.setPosition(275, y);
        item.setFillColor(sf::Color::Transparent);
        
        fileListItems_.push_back({item, entry.name});
        
        // Имя файла
        sf::Text nameText(entry.name.substr(0, 25), font_, 11);
        nameText.setFillColor(COLOR_TEXT_PRIMARY);
        nameText.setPosition(280, y + 5);
        draw(nameText);
        
        // Размер
        std::string sizeStr = std::to_string(entry.size / 1024) + " KB";
        sf::Text sizeText(sizeStr, font_, 11);
        sizeText.setFillColor(COLOR_TEXT_SECONDARY);
        sizeText.setPosition(550, y + 5);
        draw(sizeText);
        
        y += 30;
        if (y > 650) break; // Ограничение по количеству отображаемых элементов
    }
}

void MainWindow::drawButtons() {
    // Отрисовка кнопок
    for (const auto& btn : buttons_) {
        draw(btn.first);
    }
    
    // Отрисовка текстов кнопок
    for (const auto& text : texts_) {
        draw(text);
    }
}

void MainWindow::handleEvents() {
    sf::Event event;
    while (pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            close();
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(*this);
                checkButtonClick(sf::Vector2f(mousePos.x, mousePos.y));
                checkFileListClick(sf::Vector2f(mousePos.x, mousePos.y));
            }
        }
    }
}

void MainWindow::checkButtonClick(sf::Vector2f mousePos) {
    for (size_t i = 0; i < buttons_.size(); ++i) {
        if (buttons_[i].first.getGlobalBounds().contains(mousePos)) {
            // Эффект нажатия
            buttons_[i].first.setFillColor(COLOR_ACCENT);
            
            // Выполнение действия
            switch(i) {
                case 0: createNewArchive(); break;
                case 1: openArchive(); break;
                case 2: extractArchive(); break;
                case 3: addToArchive(); break;
                case 4: showAbout(); break;
            }
            return;
        }
    }
}

void MainWindow::checkFileListClick(sf::Vector2f mousePos) {
    // Обработка кликов по списку файлов
    for (size_t i = 0; i < fileListItems_.size(); ++i) {
        if (fileListItems_[i].first.getGlobalBounds().contains(mousePos)) {
            // Выделение элемента
            fileListItems_[i].first.setFillColor(sf::Color(200, 230, 200));
        }
    }
}

void MainWindow::createNewArchive() {
    // В реальной реализации здесь будет диалог выбора файлов
    std::cout << "Создание нового архива..." << std::endl;
    archiver_.createArchive("new_archive.zip", {});
}

void MainWindow::openArchive() {
    // В реальной реализации здесь будет диалог открытия файла
    std::cout << "Открытие архива..." << std::endl;
    currentArchive_ = archiver_.getArchiveInfo("test.zip");
    hasOpenArchive_ = (currentArchive_.entryCount > 0);
    refreshFileList();
}

void MainWindow::extractArchive() {
    if (!hasOpenArchive_) {
        std::cout << "Сначала откройте архив" << std::endl;
        return;
    }
    std::cout << "Распаковка архива..." << std::endl;
    archiver_.extractArchive(currentArchive_.path, "./extracted");
}

void MainWindow::addToArchive() {
    if (!hasOpenArchive_) {
        std::cout << "Сначала откройте архив" << std::endl;
        return;
    }
    std::cout << "Добавление файлов в архив..." << std::endl;
}

void MainWindow::showAbout() {
    std::cout << "PArchiver v1.0 - Современный архиватор на C++" << std::endl;
    std::cout << "Использует libarchive для работы с архивами" << std::endl;
    std::cout << "GUI на основе SFML" << std::endl;
}

void MainWindow::refreshFileList() {
    texts_.clear();
    // Пересоздание текстовых элементов
}

sf::Color MainWindow::hexToColor(const std::string& hex) {
    // Утилита для конвертации hex цвета
    return sf::Color::White;
}

void MainWindow::centerText(sf::Text& text, const sf::RectangleShape& shape) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.f,
                   textBounds.top + textBounds.height / 2.f);
    sf::FloatRect shapeBounds = shape.getGlobalBounds();
    text.setPosition(shapeBounds.left + shapeBounds.width / 2.f,
                     shapeBounds.top + shapeBounds.height / 2.f);
}

} // namespace PArchiver

int main() {
    PArchiver::MainWindow window;
    window.run();
    return 0;
}
