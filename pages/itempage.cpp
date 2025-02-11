#include "pages/itempage.h"
#include "backend/packagekithelper.h"
#include "backend/ratingshelper.h"
#include "widgets/gallery.h"
#include "widgets/stars.h"
#include "plugins/pluginloader.h"
#include <QScrollArea>
#include <DLabel>

ItemPage::ItemPage(QString app, bool snap)
{
    QScrollArea *scroll = new QScrollArea(this);
    QWidget *widget = new QWidget;
    scroll->setWidget(widget);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignTop);
    widget->setLayout(layout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
    mainLayout->addWidget(scroll);

    isSnap = snap;

    spinner = new DSpinner;
    if (isSnap) {
        PluginLoader::instance()->snapPlugin->itemPageData(this, app);
        spinner->start();
        spinner->setFixedSize(50, 50);
        layout->addWidget(spinner, 1, Qt::AlignCenter);
    } else {
        PackageKitHelper::instance()->itemPageData(this, app);
        setData(AppStreamHelper::instance()->getAppData(app));
    }
}

void ItemPage::setData(AppStreamHelper::appData data)
{
    spinner->hide();
    QHBoxLayout *header = new QHBoxLayout;
    header->setMargin(10);
    header->setAlignment(Qt::AlignVCenter);

    QLabel *icon = new QLabel;
    icon->setPixmap(data.icon.pixmap(data.icon.actualSize(QSize(64, 64))));
    header->addWidget(icon);

    QVBoxLayout *nameSection = new QVBoxLayout;
    nameSection->setMargin(0);
    nameSection->setAlignment(Qt::AlignTop);

    DLabel *name = new DLabel(data.name);
    QFont nameFont;
    nameFont.setPixelSize(22);
    name->setFont(nameFont);
    nameSection->addWidget(name);
    if (!data.developer.isNull()) {
        DLabel *developer = new DLabel(data.developer);
        nameSection->addWidget(developer);
    }
    if (RatingsHelper::instance()->averageRating(data.id) != 0) {
        stars *starsView = new stars(data.id);
        nameSection->addWidget(starsView);
    }
    header->addLayout(nameSection);

    header->addStretch();

    removeBtn = new DWarningButton;
    removeBtn->setText(tr("Uninstall"));
    removeBtn->hide();
    header->addWidget(removeBtn, 0, Qt::AlignVCenter);
    header->addSpacing(10);
    installBtn = new DSuggestButton;
    installBtn->hide();
    header->addWidget(installBtn, 0, Qt::AlignVCenter);

    layout->addLayout(header);

    progressBar = new DProgressBar;
    progressBar->hide();
    layout->addWidget(progressBar, 0, Qt::AlignTop);
    
    if (!data.screenshots.isEmpty()) {
        gallery *screenshotView = new gallery(data.screenshots);
        layout->addWidget(screenshotView);
    }

    DLabel *description = new DLabel;
    description->setWordWrap(true);
    description->setAlignment(Qt::AlignHCenter);
    description->setMargin(15);
    if (data.description.isNull()) {
        description->setText(QString("<i>%1</i>").arg(tr("No description provided.")));
    } else {
        description->setText(data.description);
    }

    layout->addWidget(description);
}

void ItemPage::setInstallButton(QString packageId, Status type, QString param)
{
    switch (type)
    {
    case NotInstalled:
        removeBtn->hide();
        progressBar->hide();
        installBtn->show();
        installBtn->setDisabled(false);
        installBtn->setText(tr("Install (%1)").arg(param));
        installBtn->disconnect(this);
        connect(installBtn, &DSuggestButton::clicked, this, [ = ] {
            if (isSnap) {
                PluginLoader::instance()->snapPlugin->install(this, packageId, false);
            } else {
                PackageKitHelper::instance()->install(this, packageId);
            }
        });
        break;
    case Launchable:
        progressBar->hide();
        installBtn->show();
        installBtn->setDisabled(false);
        installBtn->setText(tr("Open"));
        installBtn->disconnect(this);
        connect(installBtn, &DSuggestButton::clicked, this, [ = ] {
            if (isSnap) {
                PluginLoader::instance()->snapPlugin->launch(packageId);
            } else {
                PackageKitHelper::instance()->launch(packageId);
            }
        });
        removeBtn->show();
        connect(removeBtn, &DWarningButton::clicked, this, [ = ] {
            if (isSnap) {
                PluginLoader::instance()->snapPlugin->uninstall(this, packageId);
            } else {
                PackageKitHelper::instance()->uninstall(this, packageId);
            }
        });
        break;
    case Installed:
        progressBar->hide();
        installBtn->hide();
        removeBtn->show();
        connect(removeBtn, &DWarningButton::clicked, this, [ = ] {
            if (isSnap) {
                PluginLoader::instance()->snapPlugin->uninstall(this, packageId);
            } else {
                PackageKitHelper::instance()->uninstall(this, packageId);
            }
        });
        break;
    case Installing:
        progressBar->show();
        progressBar->setValue(param.toInt());
        removeBtn->hide();
        installBtn->setDisabled(true);
        break;
    }
}
