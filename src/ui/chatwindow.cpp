#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include <QIcon>
#include <QDebug>
#include "customroledialog.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWindow)
    , chatCore(new ChatCore(this))
    , voiceHandler(new VoiceHandler(this))
    , currentRole("默认")
    , roleManager(nullptr)
{
    ui->setupUi(this);
    
    // 设置按钮图标
    ui->sendButton->setIcon(QIcon(":/resources/icons/send.png"));
    ui->voiceButton->setIcon(QIcon(":/resources/icons/voice.png"));
    
    // 初始化聊天核心
    if (!chatCore->initialize()) {
        QMessageBox::critical(this, "错误", "聊天核心初始化失败");
    }
    
    // 连接信号和槽
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatWindow::on_sendButton_clicked);
    connect(ui->voiceButton, &QPushButton::clicked, this, &ChatWindow::handleVoiceButtonClicked);
    connect(ui->newRoleButton, &QPushButton::clicked, this, &ChatWindow::on_newRoleButton_clicked);
    connect(chatCore, &ChatCore::messageReceived, this, &ChatWindow::handleMessageReceived);
    connect(chatCore, &ChatCore::errorOccurred, this, &ChatWindow::handleError);
    connect(voiceHandler, &VoiceHandler::textRecognized, this, &ChatWindow::handleTextRecognized);
    connect(voiceHandler, &VoiceHandler::errorOccurred, this, &ChatWindow::handleError);
    connect(ui->roleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChatWindow::on_roleComboBox_currentIndexChanged);
    
    // 设置占位符文本
    ui->messageEdit->setPlaceholderText("请输入消息...");
    
    // 显示欢迎消息
    ui->chatDisplay->append("<div style='color: #666; font-style: italic;'>欢迎使用麒麟AI助手，请输入您的问题...</div>");
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setCurrentRole(const QString &role)
{
    currentRole = role;
    chatCore->setCurrentRole(role);
    
    // 更新下拉框选中项
    int index = ui->roleComboBox->findText(role);
    if (index != -1) {
        ui->roleComboBox->setCurrentIndex(index);
    }
}

void ChatWindow::updateRoleList(const QStringList &roles)
{
    QStringList newRoles = roles;
    // 如果没有"默认"角色，则添加
    if (!newRoles.contains("默认")) {
        newRoles.prepend("默认");
    }
    // 保存当前选中的角色
    QString currentRole = ui->roleComboBox->currentText();
    // 清空并重新添加角色列表
    ui->roleComboBox->clear();
    ui->roleComboBox->addItems(newRoles);
    // 优先选中"默认"
    int index = ui->roleComboBox->findText("默认");
    if (index != -1) {
        ui->roleComboBox->setCurrentIndex(index);
        setCurrentRole("默认");
    } else if (!newRoles.isEmpty()) {
        ui->roleComboBox->setCurrentIndex(0);
        setCurrentRole(newRoles.first());
    }
}

void ChatWindow::on_roleComboBox_currentIndexChanged(int index)
{
    if (index >= 0) {
        QString selectedRole = ui->roleComboBox->currentText();
        setCurrentRole(selectedRole);
    }
}

void ChatWindow::on_newRoleButton_clicked()
{
    
    CustomRoleDialog dialog(this); // Create a new dialog on the stack. Parent 'this' (ChatWindow) for modality.
    
    // Connect its signal for this showing only.
    connect(&dialog, &CustomRoleDialog::roleConfirmed, this, &ChatWindow::handleRoleConfirmed); 
    
    dialog.setEditMode(false); // Set mode for this new dialog.
    // A freshly constructed dialog should have empty fields by default.
    dialog.exec(); // Show modally. Blocks here.
}

void ChatWindow::handleRoleConfirmed(const RoleData &data)
{
    // 发送信号，通知新角色被创建
    emit newRoleCreated(data);
}

void ChatWindow::on_sendButton_clicked()
{
    QString message = ui->messageEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    // 显示用户消息
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->chatDisplay->append(QString("<div style='text-align: right;'><span style='color: #2196F3; font-weight: bold;'>我 (%1)</span></div>").arg(timeStr));
    ui->chatDisplay->append(QString("<div style='text-align: right; background-color: #E3F2FD; border-radius: 10px; padding: 8px; margin: 4px;'>%1</div>").arg(message));
    
    // 清空输入框
    ui->messageEdit->clear();
    
    // 发送消息到聊天核心
    chatCore->sendMessage(message, currentRole);
}

void ChatWindow::handleMessageReceived(const Message &message)
{
    // 显示AI回复
    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->chatDisplay->append(QString("<div><span style='color: #4CAF50; font-weight: bold;'>%1 (%2)</span></div>").arg(message.role).arg(timeStr));
    ui->chatDisplay->append(QString("<div style='background-color: #F1F8E9; border-radius: 10px; padding: 8px; margin: 4px;'>%1</div>").arg(message.content));
    
    // 滚动到底部
    ui->chatDisplay->verticalScrollBar()->setValue(ui->chatDisplay->verticalScrollBar()->maximum());
}

void ChatWindow::handleVoiceButtonClicked()
{
    // 检查语音处理器状态
    if (voiceHandler->isRecording()) {
        voiceHandler->stopRecording();
        ui->voiceButton->setText("开始录音");
    } else {
        if (voiceHandler->startRecording()) {
            ui->voiceButton->setText("停止录音");
        } else {
            handleError("无法启动录音");
        }
    }
}

void ChatWindow::handleTextRecognized(const QString &text)
{
    ui->messageEdit->setPlainText(text);
}

void ChatWindow::handleError(const QString &error)
{
    QMessageBox::warning(this, "错误", error);
}

void ChatWindow::onRoleSelected(const QString &roleName)
{
    setCurrentRole(roleName);
    // 清空聊天记录
    ui->chatDisplay->clear();
    // 显示欢迎消息
    QString welcomeMessage = QString("已切换到角色：%1\n\n%2")
        .arg(roleName)
        .arg("您好！我是您的AI助手，请问有什么可以帮您？");
    ui->chatDisplay->append(welcomeMessage);
} 