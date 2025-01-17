//SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
//SPDX-License-Identifier: GPL-3.0-or-later
#include "systeminfomodule.h"
#include "versionprotocolwidget.h"
#include "userlicensewidget.h"
#include "privacypolicywidget.h"
#include "hostnameitem.h"
#include "logoitem.h"
#include "widgets/titlevalueitem.h"
#include "interface/pagemodule.h"
#include "interface/vlistmodule.h"
#include "src/frame/utils.h"

#include "operation/systeminfowork.h"
#include "operation/systeminfomodel.h"
#include "widgets/widgetmodule.h"

#include <dsysinfo.h>

#include <qobject.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qwidget.h>

using namespace DCC_NAMESPACE;
DCORE_USE_NAMESPACE

SystemInfoModule::SystemInfoModule(QObject *parent)
    : HListModule(parent)
    , m_model(new SystemInfoModel(this))
    , m_work(new SystemInfoWork(m_model, this))
{
    initChildModule();
}

SystemInfoModule::~SystemInfoModule()
{
    m_model->deleteLater();
    m_work->deleteLater();
}

void SystemInfoModule::active()
{
    m_work->activate();
}

void SystemInfoModule::initChildModule()
{
    //二级菜单--关于本机
    ModuleObject *moduleAboutPc = new PageModule("aboutThisPc", tr("About This PC"), QIcon::fromTheme("dcc_on_sel"), this);
    appendChild(moduleAboutPc);

    moduleAboutPc->appendChild(new WidgetModule<LogoItem>("logo", "", this, &SystemInfoModule::initLogoModule));
    if (DSysInfo::uosType() == DSysInfo::UosType::UosServer || (DSysInfo::uosType() == DSysInfo::UosType::UosDesktop)) {
        moduleAboutPc->appendChild(new WidgetModule<HostNameItem>("hostName", tr("Computer Name"), this, &SystemInfoModule::initHostnameModule));
        moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("osName", tr("OS Name"), this, &SystemInfoModule::initOSNameModule));
        moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("version", tr("Version"), this, &SystemInfoModule::initVersionModule));
    }
    moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("edition", tr("Edition"), this, &SystemInfoModule::initEditionModule));
    moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("type", tr("Type"), this, &SystemInfoModule::initTypeModule));
    if (!IS_COMMUNITY_SYSTEM && DSysInfo::uosEditionType() != DSysInfo::UosEnterpriseC) {
        moduleAboutPc->appendChild(new WidgetModule<TitleAuthorizedItem>("authorization", tr("Authorization"), this, &SystemInfoModule::initAuthorizationModule));
    }
    moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("kernel", tr("Kernel"), this, &SystemInfoModule::initKernelModule));
    moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("processor", tr("Processor"), this, &SystemInfoModule::initProcessorModule));
    moduleAboutPc->appendChild(new WidgetModule<TitleValueItem>("memory", tr("Memory"), this, &SystemInfoModule::initMemoryModule));

    //二级菜单--协议与隐私政策
    ModuleObject *moduleAgreement = new VListModule("agreement", tr("Agreements and Privacy Policy"), QIcon::fromTheme("dcc_version"), this);

    //三级菜单--协议与隐私政策-版本协议
    ModuleObject *moduleEdition = new PageModule("editionLicense", tr("Edition License"), QIcon::fromTheme("dcc_version"), moduleAgreement);

    moduleEdition->appendChild(new WidgetModule<VersionProtocolWidget>());
    moduleAgreement->appendChild(moduleEdition);

    //三级菜单--协议与隐私政策-最终用户许可协议
    ModuleObject *moduleUserAgreement = new PageModule("endUserLicenseAgreement", tr("End User License Agreement"), QIcon::fromTheme("dcc_protocol"), moduleAgreement);
    moduleUserAgreement->appendChild(new WidgetModule<UserLicenseWidget>());
    moduleAgreement->appendChild(moduleUserAgreement);

    //三级菜单--协议与隐私政策-隐私政策
    ModuleObject *modulePolicy = new PageModule("privacyPolicy", tr("Privacy Policy"), QIcon::fromTheme("dcc_privacy_policy"), moduleAgreement);
    modulePolicy->appendChild(new WidgetModule<PrivacyPolicyWidget>());
    moduleAgreement->appendChild(modulePolicy);

    appendChild(moduleAgreement);
}

const QString systemCopyright()
{
    const QSettings settings("/etc/deepin-installer.conf", QSettings::IniFormat);
    const QString &oem_copyright = settings.value("system_info_vendor_name").toString().toLatin1();
    if (oem_copyright.isEmpty()) {
        if (IS_COMMUNITY_SYSTEM)
            return QApplication::translate("LogoModule", "Copyright© 2011-%1 Deepin Community").arg(QString(__DATE__).right(4));
        else
            return QApplication::translate("LogoModule", "Copyright© 2019-%1 UnionTech Software Technology Co., LTD").arg(QString(__DATE__).right(4));
    } else {
        return oem_copyright;
    }
}

void SystemInfoModule::initLogoModule(LogoItem *item)
{
    item->addBackground();
    item->setDescription(true); //显示文字描述
    item->setDescription(systemCopyright());//LogoItem构造函数: set the discription visible=false
    item->setLogo(DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Normal));
}

void SystemInfoModule::initHostnameModule(HostNameItem *item)
{
    item->addBackground();
    // 需要在初始化后进行设置，因为要获取width()属性
    QTimer::singleShot(0, item, [this, item] {
        item->setHostName(m_model->hostName());
    });
    connect(m_model, &SystemInfoModel::hostNameChanged, item, &HostNameItem::setHostName);
    connect(m_model, &SystemInfoModel::setHostNameError, item, &HostNameItem::onSetError);
    connect(item, &HostNameItem::hostNameChanged, m_work, &SystemInfoWork::onSetHostname);
}

void SystemInfoModule::initOSNameModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("OS Name") + ':');
    item->setValue(m_model->productName());
    connect(m_model, &SystemInfoModel::productNameChanged, item, &TitleValueItem::setValue);
}

void SystemInfoModule::initVersionModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Version") + ':');
    item->setValue(m_model->versionNumber());
    connect(m_model, &SystemInfoModel::versionNumberChanged, item, &TitleValueItem::setValue);
}

void SystemInfoModule::initEditionModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Edition") + ':');
    item->setValue(m_model->version());
    connect(m_model, &SystemInfoModel::versionChanged, item, &TitleValueItem::setValue);
}

void SystemInfoModule::initTypeModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Type") + ':');
    item->setValue(tr("%1-bit").arg(m_model->type()));
    connect(m_model, &SystemInfoModel::typeChanged, item, &TitleValueItem::setValue);
}

void setLicenseState(TitleAuthorizedItem *const authorized,  DCC_NAMESPACE::ActiveState state)
{
    if (state == Authorized) {
        authorized->setValue(QObject::tr("Activated"));
        authorized->setValueForegroundRole(QColor(21, 187, 24));
        authorized->setButtonText(QObject::tr("View"));
    } else if (state == Unauthorized) {
        authorized->setValue(QObject::tr("To be activated"));
        authorized->setValueForegroundRole(QColor(255, 87, 54));
        authorized->setButtonText(QObject::tr("Activate"));
    } else if (state == AuthorizedLapse) {
        authorized->setValue(QObject::tr("Expired"));
        authorized->setValueForegroundRole(QColor(255, 87, 54));
        authorized->setButtonText(QObject::tr("View"));
    } else if (state == TrialAuthorized) {
        authorized->setValue(QObject::tr("In trial period"));
        authorized->setValueForegroundRole(QColor(255, 170, 0));
        authorized->setButtonText(QObject::tr("Activate"));
    } else if (state == TrialExpired) {
        authorized->setValue(QObject::tr("Trial expired"));
        authorized->setValueForegroundRole(QColor(255, 87, 54));
        authorized->setButtonText(QObject::tr("Activate"));
    }
}

void SystemInfoModule::initAuthorizationModule(TitleAuthorizedItem *item)
{
    item->addBackground();
    item->setTitle(tr("Authorization") + ':');
    setLicenseState(item, m_model->licenseState());
    connect(m_model, &SystemInfoModel::licenseStateChanged, item, [item] (ActiveState state) {
        setLicenseState(item, state);
    });
    connect(item, &TitleAuthorizedItem::clicked, m_work, &SystemInfoWork::showActivatorDialog);
}

void SystemInfoModule::initKernelModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Kernel") + ':');
    item->setValue(m_model->kernel());
    connect(m_model, &SystemInfoModel::kernelChanged, item, &TitleValueItem::setValue);
}

void SystemInfoModule::initProcessorModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Processor") + ':');
    item->setValue(m_model->processor());
    connect(m_model, &SystemInfoModel::processorChanged, item, &TitleValueItem::setValue);
}

void SystemInfoModule::initMemoryModule(TitleValueItem *item)
{
    item->addBackground();
    item->setTitle(tr("Memory") + ':');
    item->setValue(m_model->memory());
    connect(m_model, &SystemInfoModel::memoryChanged, item, &TitleValueItem::setValue);
}

QString SystemInfoPlugin::name() const
{
    return QStringLiteral("systeminfo");
}

ModuleObject *SystemInfoPlugin::module()
{
    //一级菜单--系统信息
    SystemInfoModule *moduleInterface = new SystemInfoModule();
    moduleInterface->setName("systeminfo");
    moduleInterface->setDisplayName(tr("System Info"));
    moduleInterface->setDescription(tr("System Info"));
    moduleInterface->setIcon(QIcon::fromTheme("dcc_nav_systeminfo"));

    return moduleInterface;
}

QString SystemInfoPlugin::location() const
{
    return "21";
}

