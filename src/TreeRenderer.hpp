//---------------------------

#ifndef TREERENDERER_HPP
#define TREERENDERER_HPP

//---------------------------

#include <SFML/Graphics.hpp>

#include <iostream>
#include <sstream>
#include <cmath>

#include <bitset>

#include "Map.hpp"

//---------------------------

template <class Key>
class TreeRenderedItem : public sf::Drawable {
public:

    TreeRenderedItem(const Key& _key) : key(_key) {
        //
    }

    int level  = 0,
        offset = 0;

    sf::Vertex vertices[4];
    sf::Text sign;

    const Key& key;

private:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(vertices, 4, sf::PrimitiveType::TriangleFan, states);
        target.draw(sign, states);
    }

};

//---------------------------

template <class Key, class Data>
class TreeRenderer : public sf::Drawable, public sf::Transformable {
public:

    //---------------------------

    enum class State {
        TreeView = 0,
        HelpScreen,
        AddItemKey,
        AddItemData,
        CountingByData,

    };

    //---------------------------

    TreeRenderer() {

        m_sign.setFillColor(sf::Color::Black);
        m_helpScreenSign.setFillColor(sf::Color::Black);

        this->setControlKeySign("W) up\nS) down\nA) left\nD) right\nQ) remove\nE) add\nR) count items by data\nF1) inorder, preorder, postorder - print\nF2) Horizontal and Vertical print", "F", "Tab", "Enter");
        this->setSize(450.0f, 320.0f);
        this->setBackgroundColor(sf::Color(128, 128, 128));
        this->deactivate();

        m_helpScreen[0].color = sf::Color(255, 255, 255, 240);
        m_helpScreen[1].color = m_helpScreen[0].color;
        m_helpScreen[2].color = m_helpScreen[0].color;
        m_helpScreen[3].color = m_helpScreen[0].color;

    }

    //---------------------------

    void setFont(const sf::Font& font) {

        m_sign.setFont(font);
        m_helpScreenSign.setFont(font);

        for(size_t i = 0; i < m_items.size(); ++i)
            m_items[i]->sign.setFont(font);

        this->setupSignBounds();
    }

    //---------------------------

    void setBackgroundColor(const sf::Color& color) {

        m_background[0].color = color;
        m_background[1].color = color;
        m_background[2].color = color;
        m_background[3].color = color;
    }

    //---------------------------

    void setSize(float width, float height) {

        m_size.x = width;
        m_size.y = height;

        m_background[1].position.x = m_size.x;
        m_background[2].position = m_size;
        m_background[3].position.y = m_size.y;

        this->setupHelpScreenBounds();
        this->setupHelpScreenSignBounds();
        this->resizeItems();
        this->setupSignBounds();

    }

    //---------------------------

    void setSize(const sf::Vector2f& size) {
        this->setSize(size.x, size.y);
    }

    //---------------------------

    void setControlKeySign(const std::string& helpSign, const std::string& keyHelp, const std::string& keySwitchKeyDataEdit, const std::string& keyFinishEdit) {

        m_helpSign = helpSign;
        m_keyHelp = keyHelp;
        m_keySwitchKeyDataEdit = keySwitchKeyDataEdit;
        m_keyFinishEdit = keyFinishEdit;
        m_helpScreenSign.setString(m_helpSign);

        if(m_isActive)
            m_sign.setString(m_keyHelp);

        this->setupHelpScreenSignBounds();
    }

    //---------------------------

    //---------------------------
    // Control section
    //---------------------------

    //---------------------------

    ///*horz* = <0 -> move to the left, *horz* = >0 -> move to the right
    void moveSelection(int horz, int vert) {

        if(!m_isActive || m_state != State::TreeView)
            return;

        if(m_selectedItem != nullptr) {

            int targetLevel = std::min(std::max(0, m_selectedItem->level + vert), m_maxLevel - 1),
                targetOffset = m_selectedItem->offset;

            if(vert > 0)
                targetOffset <<= vert;

            else if(vert < 0)
                targetOffset >>= -vert;

            targetOffset += horz;

            for(size_t i = 0; i < m_items.size(); ++i) {
                if(m_items[i]->level == targetLevel && (m_items[i]->offset == targetOffset || m_items[i]->offset == targetOffset + 1)) {

                    m_selectedItem = m_items[i];
                    m_animClock.restart();

                    break;
                }
            }

        } else if(m_items.size() > 0) {

            m_selectedItem = m_items[0];
            m_animClock.restart();
        }

    }

    //---------------------------

    void clearSelection() {
        if(!m_isActive)
            return;

        m_selectedItem = nullptr;
    }

    //---------------------------

    const Key& getSelectedItemKey() const {
        return m_selectedItem == nullptr ? m_emptyKey : m_selectedItem->key;
    }

    //---------------------------

    void openHelpMenu() {
        if(!m_isActive)
            return;

        m_state = State::HelpScreen;
        m_helpScreenSign.setString(m_helpSign);

        this->setupHelpScreenBounds();
    }

    //---------------------------

    void closeHelpMenu() {
        if(!m_isActive)
            return;

        m_state = State::TreeView;

        this->setupHelpScreenBounds();
    }

    //---------------------------

    void toggleHelpMenu() {
        if(m_state == State::HelpScreen)
            this->closeHelpMenu();
        else
            this->openHelpMenu();
    }

    //---------------------------

    void activate() {
        if(m_isActive)
            return;

        m_isActive = true;
        m_sign.setString(m_keyHelp);

        this->setupSignBounds();
    }

    //---------------------------

    void deactivate() {
        m_isActive = false;
        m_sign.setString("");
    }

    //---------------------------

    //---------------------------
    // Add Item control section
    //---------------------------

    //---------------------------

    bool isEditState() const {
        return m_state == State::AddItemKey || m_state == State::AddItemData;
    }

    //---------------------------

    void startAddItem() {
        if(!m_isActive)
            return;

        m_state = State::AddItemKey;
        m_tmpValue.clear();

        this->setupAddItemPrintingText();
        this->setupHelpScreenBounds();
    }

    //---------------------------

    void startAddItemDataEdit() {
        if(!m_isActive)
            return;

        try {

            m_tmpNewItem.first = std::stoll(m_tmpValue);
            m_state = State::AddItemData;
            m_tmpValue.clear();

            this->setupAddItemPrintingText();

        } catch(std::exception& e) {

            std::cerr << "[Exception] Failed to parse Key: " << e.what() << std::endl;
            m_tmpValue.clear();
            this->finishNewItemEdit();

        }
        //std::cout << "Cur: " << ("Key: " + std::to_string(m_tmpNewItem.first) + "\nData: " + m_tmpValue) << std::endl;
    }

    //---------------------------

    void popChar() {

        if(!m_isActive || (!this->isEditState() && !this->isFindingState()))
            return;

        m_tmpValue.back() = '\0';
        m_tmpValue.resize(m_tmpValue.size() > 0 ? m_tmpValue.size() - 1 : 0);

        if(this->isEditState())
            this->setupAddItemPrintingText();
        else
            this->setupCountItemPrintingText();
    }

    //---------------------------

    void addChar(char symbol) {
        if(!m_isActive || (!this->isEditState() && !this->isFindingState()))
            return;

        m_tmpValue += symbol;

        //std::cout << "String: " << m_tmpValue << std::endl;
        if(this->isEditState())
            this->setupAddItemPrintingText();
        else
            this->setupCountItemPrintingText();
    }

    //---------------------------

    void finishNewItemEdit() {
        if(!m_isActive || !this->isEditState())
            return;

        m_tmpNewItem.second = m_tmpValue[0];

        this->closeHelpMenu();

        m_tmpValue.clear();
    }

    //---------------------------

    const std::pair<Key, Data>& getPendingItem() const {
        return m_tmpNewItem;
    }

    //---------------------------

    //---------------------------
    // Count by data control section
    //---------------------------

    //---------------------------

    bool isFindingState() const {
        return m_state == State::CountingByData;
    }

    //---------------------------

    void startCounting() {
        if(!m_isActive)
            return;

        m_state = State::CountingByData;
        m_tmpValue.clear();
        m_nFoundItems = m_emptyFoundResult;

        this->setupCountItemPrintingText();
        this->setupHelpScreenBounds();
    }

    //---------------------------

    void setNumFoundItems(size_t nFound) {

        m_nFoundItems = nFound;

        this->setupCountItemPrintingText();
    }

    //---------------------------

    void stopCounting() {
        if(!m_isActive || !this->isFindingState())
            return;

        this->closeHelpMenu();

        m_tmpValue.clear();
        m_nFoundItems = m_emptyFoundResult;
    }

    //---------------------------

    const Data& getCountingData() const {
        return isFindingState() ? m_tmpValue[0] : m_emptyData;
    }

    //---------------------------

    void clear() {

        for(size_t i = 0; i < m_items.size(); ++i)
            delete m_items[i];

        m_items.clear();
    }

    //---------------------------

    void buildFromVector(const std::vector<DataS<Key, Data>>& tree) {

        this->clear();
        this->clearSelection();

        m_maxLevel = 0;
        for(size_t i = 0; i < tree.size(); ++i)
            m_maxLevel = std::max(m_maxLevel, tree[i].level);
        ++m_maxLevel;

        std::vector<int> offsets;
        offsets.resize(m_maxLevel);

        int currLevel = 0;

        std::stringstream keyDataPair;
        for(size_t i = 0; i < tree.size(); ++i) {

            const DataS<Key, Data>& curr = tree[i];
            TreeRenderedItem<Key>* item = new TreeRenderedItem<Key>(curr.node->key);

            int offset;
            if(curr.level < currLevel) { // One tree's part is done -> next one

                currLevel = curr.level;
                offsets[currLevel] += 1;
                offset = offsets[currLevel];

            } else if(curr.level > currLevel) { // Next level (make offset according to "side")

                offsets[curr.level] = offsets[currLevel] * 2;
                offset = offsets[curr.level] + std::max(0, curr.state - 1);
                currLevel = curr.level;

            } else {

                offsets[currLevel] += std::max(0, curr.state - 1);
                offset = offsets[currLevel];

            }

            keyDataPair.str("");
            keyDataPair << curr.node->key;
            keyDataPair << ": ";
            keyDataPair << curr.node->data;

            item->sign.setString(keyDataPair.str());
            item->level = curr.level;
            item->offset = offset;

            //std::cout << curr.level << " " << curr.state << " " << std::bitset<8>(item->offset) << std::endl;

            if(m_sign.getFont() != nullptr)
                item->sign.setFont(*m_sign.getFont());

            m_items.push_back(item);
        }

        this->resizeItems();
    }

    //---------------------------

    ~TreeRenderer() {
        this->clear();
    }

    //---------------------------

private:

    static const Key m_emptyKey;
    static const Data m_emptyData;

    const size_t m_emptyFoundResult = static_cast<size_t>(-1);

    sf::Vector2f m_size;
    std::vector<TreeRenderedItem<Key>*> m_items;

    const TreeRenderedItem<Key>* m_selectedItem = nullptr;
    int m_maxLevel = 1;

    bool m_isActive = false;

    sf::Vertex m_background[4];
    sf::Clock m_animClock;
    sf::Text m_sign;

    State m_state = State::TreeView;
    sf::Vertex m_helpScreen[4];
    sf::Text m_helpScreenSign;

    std::string m_tmpValue,
                m_helpSign,
                m_keyHelp,
                m_keySwitchKeyDataEdit,
                m_keyFinishEdit;

    std::pair<Key, Data> m_tmpNewItem;

    size_t m_nFoundItems = m_emptyFoundResult;

    /*
     * Vertex render order:
     *
     * 0-------1
     * |       |
     * |       |
     * |       |
     * 3-------2
     *
     */
    void resizeItems() {

        sf::Vector2f currSize(m_size.x / this->getScale().x, m_size.y / this->getScale().y);
        float itemHeight = currSize.y / m_maxLevel;

        for(size_t i = 0; i < m_items.size(); ++i) {

            float itemWidth = currSize.x / (1 << m_items[i]->level);
            unsigned char comp = (m_items[i]->level + 1) * 64 / m_maxLevel;

            m_items[i]->vertices[0].position.x = m_items[i]->offset * itemWidth;
            m_items[i]->vertices[0].position.y = m_items[i]->level * itemHeight;

            m_items[i]->vertices[1].position.x = m_items[i]->vertices[0].position.x + itemWidth;
            m_items[i]->vertices[1].position.y = m_items[i]->vertices[0].position.y;

            m_items[i]->vertices[2].position.x = m_items[i]->vertices[1].position.x;
            m_items[i]->vertices[2].position.y = m_items[i]->vertices[1].position.y + itemHeight;

            m_items[i]->vertices[3].position.x = m_items[i]->vertices[0].position.x;
            m_items[i]->vertices[3].position.y = m_items[i]->vertices[2].position.y;

            m_items[i]->vertices[0].color = m_items[i]->offset & 1 ? sf::Color(comp, comp, comp) : sf::Color(200, comp, comp);
            m_items[i]->vertices[1].color = m_items[i]->vertices[0].color;
            m_items[i]->vertices[2].color = m_items[i]->vertices[0].color;
            m_items[i]->vertices[3].color = m_items[i]->vertices[0].color;

            m_items[i]->sign.setPosition((m_items[i]->vertices[0].position.x + m_items[i]->vertices[2].position.x) * 0.5f,
                                         (m_items[i]->vertices[0].position.y + m_items[i]->vertices[2].position.y) * 0.5f);
            m_items[i]->sign.setOrigin((m_items[i]->sign.getLocalBounds().width - m_items[i]->sign.getLocalBounds().left) * 0.5f,
                                        m_items[i]->sign.getLocalBounds().height * 0.5f + m_items[i]->sign.getLocalBounds().top);

        }
    }

    //---------------------------

    void setupHelpScreenBounds() {

        if(m_state == State::TreeView) {

            m_helpScreen[1].position.x = std::max(m_size.x * 0.1f, m_size.y * 0.1f);
            m_helpScreen[2].position.x = m_helpScreen[1].position.x;
            m_helpScreen[2].position.y = 0.0f;
            m_helpScreen[3].position.y = m_helpScreen[1].position.x;

        } else {

            m_helpScreen[1].position.x = m_size.x;
            m_helpScreen[2].position = m_size;
            m_helpScreen[3].position.y = m_size.y;
        }
    }

    //---------------------------

    void setupHelpScreenSignBounds() {
        m_helpScreenSign.setPosition(std::max(m_size.x * 0.1f, m_size.y * 0.1f), 0.0f);
    }

    //---------------------------

    void setupSignBounds() {

        float min = std::max(m_size.x * 0.1f, m_size.y * 0.1f) * 0.25f;

        m_sign.setPosition(min, min);
        m_sign.setOrigin(m_sign.getLocalBounds().width * 0.5f + m_sign.getLocalBounds().left,
                         m_sign.getLocalBounds().height * 0.5f + m_sign.getLocalBounds().top);
    }

    //---------------------------

    void setupAddItemPrintingText() {

        if(m_state == State::AddItemKey)
            m_helpScreenSign.setString("New item key: " + m_tmpValue + "\n(press \"" + m_keySwitchKeyDataEdit + "\" to confirm)");

        else if(m_state == State::AddItemData)
            m_helpScreenSign.setString("New item key: " + std::to_string(m_tmpNewItem.first) + "\nData: " + m_tmpValue + "\n(press \"" + m_keyFinishEdit + "\" to add a new item)");
    }

    //---------------------------

    void setupCountItemPrintingText() {
        std::string msg = "Count by data: " + m_tmpValue;

        if(m_nFoundItems == m_emptyFoundResult)
            msg += "\nFound: ---";
        else
            msg += "\nFound: " + std::to_string(m_nFoundItems);
        msg += "\n(press \"" + m_keySwitchKeyDataEdit + "\" to count, \"" + m_keyFinishEdit + "\" to exit)";

        m_helpScreenSign.setString(msg);
    }

    //---------------------------

    void draw(sf::RenderTarget& target, sf::RenderStates states) const {

        states.transform.combine(this->getTransform());
        target.draw(m_background, 4, sf::PrimitiveType::TriangleFan, states);

        for(size_t i = 0; i < m_items.size(); ++i) {

            sf::Color c = m_items[i]->vertices[0].color;

            if(m_items[i] == m_selectedItem) {

                float s  = std::abs(std::cos(m_animClock.getElapsedTime().asSeconds())) * 0.7f + 0.3f,
                      is = 1.0f - s;

                sf::Color c2 = c;
                c2.r = static_cast<unsigned char>(std::min(0   * s + c.r * is, 255.0f));
                c2.g = static_cast<unsigned char>(std::min(170 * s + c.g * is, 255.0f));
                c2.b = static_cast<unsigned char>(std::min(255 * s + c.b * is, 255.0f));

                m_items[i]->vertices[0].color = c2;
                m_items[i]->vertices[1].color = c2;
                m_items[i]->vertices[2].color = c2;
                m_items[i]->vertices[3].color = c2;
            }

            target.draw(*m_items[i], states);

            if(m_items[i] == m_selectedItem) {

                m_items[i]->vertices[0].color = c;
                m_items[i]->vertices[1].color = c;
                m_items[i]->vertices[2].color = c;
                m_items[i]->vertices[3].color = c;
            }
        }

        target.draw(m_helpScreen, 4, sf::PrimitiveType::TriangleFan);
        target.draw(m_sign, states);

        if(m_state != State::TreeView)
            target.draw(m_helpScreenSign, states);

    }

    //---------------------------

};

//---------------------------

template <class Key, class Data>
const Key TreeRenderer<Key, Data>::m_emptyKey = Key();

//---------------------------

template <class Key, class Data>
const Data TreeRenderer<Key, Data>::m_emptyData = Data();

//---------------------------

#endif // TREERENDERER_HPP

//---------------------------
