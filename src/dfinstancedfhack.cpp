#include "dfinstancedfhack.h"

#include <QTextCodec>

constexpr char RemoteMemAccess::module[];
constexpr char RemoteMemAccess::info[];
constexpr char RemoteMemAccess::read_raw[];
constexpr char RemoteMemAccess::read_string[];
constexpr char RemoteMemAccess::write_raw[];
constexpr char RemoteMemAccess::write_string[];

DFInstanceDFHack::DFInstanceDFHack(QObject *parent)
    : DFInstance(parent)
    , m_connected(false)
    , m_dfhack_client()
    , m_core_suspend(&m_dfhack_client)
    , m_core_resume(&m_dfhack_client)
    , m_remote_info(&m_dfhack_client)
    , m_remote_read_raw(&m_dfhack_client)
    , m_remote_read_string(&m_dfhack_client)
    , m_remote_write_raw(&m_dfhack_client)
    , m_remote_write_string(&m_dfhack_client)
{
    for (const auto notifier: {
            &m_core_suspend.call_notifier,
            &m_core_resume.call_notifier,
            &m_remote_info.call_notifier,
            &m_remote_read_raw.call_notifier,
            &m_remote_read_string.call_notifier,
            &m_remote_write_raw.call_notifier,
            &m_remote_write_string.call_notifier}) {
        connect(notifier, &DFHack::CallNotifier::notification,
                this, &DFInstanceDFHack::log_text);
    }
    connect(&m_dfhack_client, &DFHack::Client::connectionChanged,
            this, &DFInstanceDFHack::connection_changed);
    connect(&m_dfhack_client, &DFHack::Client::socketError,
            this, &DFInstanceDFHack::connection_error);
}

DFInstanceDFHack::~DFInstanceDFHack()
{
}

void DFInstanceDFHack::find_running_copy()
{
    m_dfhack_client.disconnect();
    m_status = DFS_DISCONNECTED;

    QEventLoop event_loop;
    connect(&m_dfhack_client, &DFHack::Client::connectionChanged,
            &event_loop, &QEventLoop::quit);
    connect(&m_dfhack_client, &DFHack::Client::socketError,
            &event_loop, &QEventLoop::quit);
    if (!m_dfhack_client.connect("localhost", DFHack::Client::DefaultPort))
        return;
    event_loop.exec();
    if (!m_connected)
        return;
    if (!m_core_suspend.bind().get()) {
        LOGE << "Failed to bind suspend";
        return;
    }
    if (!m_core_resume.bind().get()) {
        LOGE << "Failed to bind resume";
        return;
    }
    if (!m_remote_info.bind().get()) {
        LOGE << "Failed to bind Info";
        return;
    }
    if (!m_remote_read_raw.bind().get()) {
        LOGE << "Failed to bind ReadString";
        return;
    }
    if (!m_remote_read_string.bind().get()) {
        LOGE << "Failed to bind ReadString";
        return;
    }
    if (!m_remote_write_raw.bind().get()) {
        LOGE << "Failed to bind WriteRaw";
        return;
    }
    if (!m_remote_write_string.bind().get()) {
        LOGE << "Failed to bind WriteString";
        return;
    }

    if (m_remote_info.call().get() != DFHack::CommandResult::Ok) {
        LOGE << "Failed to get information about DF instance.";
        return;
    }

    switch (m_remote_info.out.arch()) {
    case dfproto::RemoteMemAccess::I386:
        m_pointer_size = 4;
        break;
    case dfproto::RemoteMemAccess::AMD64:
        m_pointer_size = 8;
        break;
    default:
        LOGE << "Invalid arch";
        m_pointer_size = sizeof(VIRTADDR);
    }

    m_status = DFS_CONNECTED;
    set_memory_layout(QString("0x%1").arg(QString::fromStdString(m_remote_info.out.checksum()).left(8)));
}

bool DFInstanceDFHack::attach()
{
    LOGT << "STARTING ATTACH" << m_attach_count;
    if (is_attached()) {
        m_attach_count++;
        LOGT << "ALREADY ATTACHED SKIPPING..." << m_attach_count;
        return true;
    }

    m_core_suspend.call().get();

    m_attach_count++;
    LOGT << "FINISHED ATTACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceDFHack::detach() {
    //LOGT << "STARTING DETACH" << m_attach_count;
    m_attach_count--;
    if (m_attach_count > 0) {
        LOGT << "NO NEED TO DETACH SKIPPING..." << m_attach_count;
        return true;
    }

    m_core_resume.call().get();
    LOGT << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceDFHack::df_running()
{
    return m_connected;
}

USIZE DFInstanceDFHack::read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer)
{
    m_remote_read_raw.in.set_address(addr);
    m_remote_read_raw.in.set_length(bytes);
    if (m_remote_read_raw.call().get() != DFHack::CommandResult::Ok) {
        LOGE << "Failed to read" << bytes << "bytes from" << hexify(addr);
        memset(buffer, 0, bytes);
        return 0;
    }
    const auto &data = m_remote_read_raw.out.data();
    memcpy(buffer, data.data(), data.size());
    return data.size();
}

USIZE DFInstanceDFHack::write_raw(const VIRTADDR addr, const USIZE bytes, const void *buffer)
{
    m_remote_write_raw.in.set_address(addr);
    m_remote_write_raw.in.mutable_data()->assign(reinterpret_cast<const char *>(buffer), bytes);
    if (m_remote_write_raw.call().get() != DFHack::CommandResult::Ok) {
        LOGE << "Failed to write" << bytes << "bytes to" << hexify(addr);
        return 0;
    }
    return bytes;
}

QString DFInstanceDFHack::read_string(const VIRTADDR addr)
{
    m_remote_read_string.in.set_address(addr);
    if (m_remote_read_string.call().get() != DFHack::CommandResult::Ok) {
        LOGE << "Failed to read string from" << hexify(addr);
        return QString();
    }
    const auto &data = m_remote_read_string.out.data();
    return QTextCodec::codecForName("IBM437")->toUnicode(data.data(), data.size());
}

USIZE DFInstanceDFHack::write_string(const VIRTADDR addr, const QString &str)
{
    m_remote_write_string.in.set_address(addr);
    m_remote_write_string.in.set_data(QTextCodec::codecForName("IBM437")->fromUnicode(str).toStdString());
    if (m_remote_write_string.call().get() != DFHack::CommandResult::Ok) {
        LOGE << "Failed to write string" << str << "to" << hexify(addr);
        return 0;
    }
    return str.size();
}

void DFInstanceDFHack::log_text(DFHack::Color, const QString &text) const
{
    LOGI << "DFHack:" << text;
}

void DFInstanceDFHack::connection_changed(bool connected)
{
    LOGI << "DFHack connection changed:" << connected;
    m_connected = connected;
}

void DFInstanceDFHack::connection_error(QAbstractSocket::SocketError error, const QString &error_string)
{
    LOGE << "DFHack connection error:" << error_string;
    m_connected = false;
}

