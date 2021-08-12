#include "addresswidget.h"

#include <QRegularExpression>
#include <QSortFilterProxyModel>

AddressWidget::AddressWidget(QWidget* parent)
    : QTabWidget(parent)
    , table(new TableModel(this))
    , newAddressTab(new NewAddressTab(this))
{
    connect(newAddressTab, &NewAddressTab::sendDetails,
        this, &AddressWidget::addEntry);
    addTab(newAddressTab, tr("Address Book"));
    setupTabs();
}

void AddressWidget::setupTabs()
{
    const auto groups = { "ABC", "DEF", "GHI", "JKL", "MNO", "PQR", "STU", "VW", "XYZ" };
    for (const QString& str : groups) {
        //开头为str中任意字符的字符串.
        const auto regExp = QRegularExpression(QString("^[%1].*").arg(str), QRegularExpression::CaseInsensitiveOption);
        //在Model/View模式的编程中, ProxyModel可以用来进行数据的排序过滤等操作.
        auto proxyModel =new QSortFilterProxyModel(this);

    }
}
