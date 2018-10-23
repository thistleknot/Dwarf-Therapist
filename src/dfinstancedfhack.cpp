#include "dfinstancedfhack.h"

#include <QTextCodec>

constexpr char RemoteMemAccess::module[];
constexpr char RemoteMemAccess::info[];
constexpr char RemoteMemAccess::read_raw[];
constexpr char RemoteMemAccess::read_string[];
constexpr char RemoteMemAccess::write_raw[];
constexpr char RemoteMemAccess::write_string[];

constexpr std::size_t DFInstanceDFHack::PageSize;

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
    if (m_connected) {
        m_dfhack_client.disconnect();
        QEventLoop event_loop;
        connect(&m_dfhack_client, &DFHack::Client::connectionChanged,
                &event_loop, &QEventLoop::quit);
        connect(&m_dfhack_client, &DFHack::Client::socketError,
                &event_loop, &QEventLoop::quit);
        if (m_dfhack_client.disconnect())
            event_loop.exec();
    }
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

    m_page_cache.clear();
    m_read_raw_count = 0;
    m_read_string_count = 0;
    m_page_download_count = 0;
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

    m_page_cache.clear();
    LOGD << "read_raw count:" << m_read_raw_count;
    LOGD << "read_string count:" << m_read_string_count;
    LOGD << "page downloaded:" << m_page_download_count;
    m_core_resume.call().get();
    LOGT << "FINISHED DETACH" << m_attach_count;
    return m_attach_count > 0;
}

bool DFInstanceDFHack::df_running()
{
    return m_connected;
}

const std::array<char, DFInstanceDFHack::PageSize> &DFInstanceDFHack::get_page(VIRTADDR page_addr)
{
    auto it = m_page_cache.lower_bound(page_addr);
    if (it->first != page_addr) {
        ++m_page_download_count;
        it = m_page_cache.emplace_hint(it, std::piecewise_construct,
                                       std::forward_as_tuple(page_addr),
                                       std::forward_as_tuple());
        m_remote_read_raw.in.set_address(page_addr);
        m_remote_read_raw.in.set_length(PageSize);
        if (m_remote_read_raw.call().get() != DFHack::CommandResult::Ok) {
            LOGE << "Failed to read" << PageSize << "bytes from" << hexify(page_addr);
        }
        else {
            std::copy(m_remote_read_raw.out.data().begin(), m_remote_read_raw.out.data().end(),
                      it->second.begin());
        }
    }
    return it->second;
}

USIZE DFInstanceDFHack::read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer)
{
    ++m_read_raw_count;
    for (auto page_addr = addr & -PageSize; addr+bytes > page_addr; page_addr+=PageSize) {
        const auto &page = get_page(page_addr);
        if (addr > page_addr)
            memcpy(buffer, &page[addr-page_addr],
                   std::min<std::size_t>(bytes, page_addr+PageSize-addr));
        else
            memcpy(reinterpret_cast<char *>(buffer)+page_addr-addr, &page[0],
                   std::min<std::size_t>(addr+bytes-page_addr, PageSize));
    }
    return bytes;
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

static std::pair<VIRTADDR, std::size_t> check_string (DFInstance *df, VIRTADDR addr) {
    constexpr auto error_pair = std::make_pair<VIRTADDR, std::size_t> (0, 0);
    auto data_addr = df->read_addr(addr);
    if (!data_addr)
        return error_pair;
    auto pointer_size = df->pointer_size();
    std::vector<char> rep(3 * pointer_size);
    if (!df->read_raw(data_addr-rep.size(), rep.size(), rep.data()))
        return error_pair;
    std::size_t length = 0, capacity = 0;
    std::copy_n(&rep[0], pointer_size, reinterpret_cast<char *>(&length));
    std::copy_n(&rep[pointer_size], pointer_size, reinterpret_cast<char *>(&capacity));
    return length <= capacity ? std::make_pair(data_addr, length) : error_pair;
}

QString DFInstanceDFHack::read_string(const VIRTADDR addr)
{
    auto data = check_string(this, addr);
    if (data.first) {
        std::vector<char> buffer(data.second);
        read_raw(data.first, buffer.size(), buffer.data());

        return QTextCodec::codecForName("IBM437")->toUnicode(buffer.data(), buffer.size());
    }
    else {
        LOGE << "Invalid string at" << hexify(addr);
        return QString ();
    }
    // dead code
    //++m_read_string_count;
    //m_remote_read_string.in.set_address(addr);
    //if (m_remote_read_string.call().get() != DFHack::CommandResult::Ok) {
    //    LOGE << "Failed to read string from" << hexify(addr);
    //    return QString();
    //}
    //const auto &data = m_remote_read_string.out.data();
    //return QTextCodec::codecForName("IBM437")->toUnicode(data.data(), data.size());
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

