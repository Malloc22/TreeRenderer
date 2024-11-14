//---------------------------

#include <iostream>

#include "Map.hpp"
#include "TreeRenderer.hpp"

//---------------------------

#define IDENT_PRINT printf("\n--------------------------\n\n")

//---------------------------

int main() {

    Map<int, char> map;

    map.add(1, 'a');
    map.add(2, 'b');
    map.add(3, 'c');
    map.add(4, 'd');
    map.add(5, 'e');

    for(uint8_t i = 6; i < 15; ++i)
        map.add(i, 'a' + i);

    IDENT_PRINT;

    map.debugPrint();

    IDENT_PRINT;

    std::cout << "count data: c\n";
    std::cout << map.getCountElement('c') << std::endl;

    IDENT_PRINT;

    std::cout << "remove key: 1\n";

    map.remove(1);
    map.debugPrint();

    IDENT_PRINT;

    std::cout << "inorder: " << map.inorder() << std::endl;
    std::cout << "preorder: " << map.preorder() << std::endl;
    std::cout << "postorder: " << map.postorder() << std::endl;

    IDENT_PRINT;

    std::cout << "PrintVertical: " << std::endl;
    std::cout << map.getPrintVertical() << std::endl;

    IDENT_PRINT;

    std::cout << "PrintHorizontal: " << std::endl;
    std::cout << map.getPrintHorizontal() << std::endl;

    IDENT_PRINT;

    std::vector<DataS<int, char>> buff = map.getTree();

    std::cout << "Enter to continue for render tree\n";
    getchar();

    system("cls");
    std::cout << "init renderer\n";

    sf::RenderWindow window(sf::VideoMode(640, 480), "Tree renderer", sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    sf::Font font;
    if(!font.loadFromFile("resources/Arialuni.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        map.clear();
        return 1;
    }

    TreeRenderer<int, char> renderer;
    renderer.setSize(640, 480);
    renderer.setFont(font);
    renderer.buildFromVector(buff);
    renderer.activate();

    while(window.isOpen()) {

        sf::Event event;
        while(window.pollEvent(event)) {

            if(event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();

            } else if(event.type == sf::Event::KeyReleased) {

                if(renderer.isEditState()) {

                    if(event.key.code == sf::Keyboard::Tab)
                        renderer.startAddItemDataEdit();

                    else if(event.key.code == sf::Keyboard::Enter) {
                        renderer.finishNewItemEdit();
                        map.add(renderer.getPendingItem());
                        renderer.buildFromVector(map.getTree());
                    }

                } else if(renderer.isFindingState()) {

                    if(event.key.code == sf::Keyboard::Tab)
                        renderer.setNumFoundItems(map.getCountElement(renderer.getCountingData()));

                    else if(event.key.code == sf::Keyboard::Enter)
                        renderer.stopCounting();

                } else {
                    if(event.key.code == sf::Keyboard::W)
                        renderer.moveSelection(0, -1);

                    else if(event.key.code == sf::Keyboard::S)
                        renderer.moveSelection(0, 1);

                    else if(event.key.code == sf::Keyboard::A)
                        renderer.moveSelection(-1, 0);

                    else if(event.key.code == sf::Keyboard::D)
                        renderer.moveSelection(1, 0);

                    else if(event.key.code == sf::Keyboard::F)
                        renderer.toggleHelpMenu();

                    else if(event.key.code == sf::Keyboard::E)
                        renderer.startAddItem();

                    else if(event.key.code == sf::Keyboard::R)
                        renderer.startCounting();

                    else if(event.key.code == sf::Keyboard::Q) {
                        map.remove(renderer.getSelectedItemKey());
                        renderer.buildFromVector(map.getTree());
                    }

                    else if(event.key.code == sf::Keyboard::F1) {
                        system("cls");
                        IDENT_PRINT;
                        std::cout << "inorder: " << map.inorder() << std::endl;
                        std::cout << "preorder: " << map.preorder() << std::endl;
                        std::cout << "postorder: " << map.postorder() << std::endl;
                        IDENT_PRINT;
                    }

                    else if(event.key.code == sf::Keyboard::F2) {
                        system("cls");
                        IDENT_PRINT;
                        std::cout << "PrintHorizontal: " << map.getPrintHorizontal() << std::endl;
                        IDENT_PRINT;
                        std::cout << "PrintVertical: " << map.getPrintVertical() << std::endl;
                        IDENT_PRINT;
                    }
                }

            } else if(event.type == sf::Event::TextEntered) {
                //std::cout << "Key: " << event.text.unicode << std::endl;
                if(event.text.unicode == 8) // Backspace
                    renderer.popChar();

                else if(event.text.unicode != 13 && event.text.unicode != 9) // Not an Enter nor Tab
                    renderer.addChar(event.text.unicode);

            } else if(event.type == sf::Event::Resized) {
                sf::View v = window.getView();

                v.setSize(event.size.width, event.size.height);
                v.setCenter(v.getSize() * 0.5f);

                renderer.setSize(v.getSize());
                window.setView(v);
            }
        }

        window.clear();
        window.draw(renderer);
        window.display();
    }

    map.clear();

    return 0;
}
