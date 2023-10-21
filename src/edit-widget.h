#include "pch.h"

class EditOutputWidget : virtual public QDialog {
public:
    virtual ~EditOutputWidget() {}
};

EditOutputWidget* createEditOutputWidget(const std::string& targetid, QWidget* parent = 0);

class SaveOutputWidget : virtual public QDialog {
public:
    virtual ~SaveOutputWidget() {}
};

SaveOutputWidget* createSaveOutputWidget(const std::string& targetid, QWidget* parent = 0);

