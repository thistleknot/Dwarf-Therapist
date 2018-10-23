#ifndef DFINSTANCE_DFHACK_H
#define DFINSTANCE_DFHACK_H
#include "dfinstance.h"

#include <dfhack-client-qt/Client.h>
#include <dfhack-client-qt/Core.h>
#include <dfhack-client-qt/Function.h>
#include "remotememaccess.pb.h"

class RemoteMemAccess {
    static constexpr char module[] = "remotememaccess";
    static constexpr char info[] = "Info";
    static constexpr char read_raw[] = "ReadRaw";
    static constexpr char read_string[] = "ReadString";
    static constexpr char write_raw[] = "WriteRaw";
    static constexpr char write_string[] = "WriteString";
public:
    using Info = DFHack::Function<module, info,
                                  dfproto::EmptyMessage,
                                  dfproto::RemoteMemAccess::Info>;
    using ReadRaw = DFHack::Function<module, read_raw,
                                     dfproto::RemoteMemAccess::ReadRawIn,
                                     dfproto::RemoteMemAccess::ReadOut>;
    using ReadString = DFHack::Function<module, read_string,
                                        dfproto::RemoteMemAccess::ReadStringIn,
                                        dfproto::RemoteMemAccess::ReadOut>;
    using WriteRaw = DFHack::Function<module, write_raw,
                                      dfproto::RemoteMemAccess::WriteIn,
                                      dfproto::EmptyMessage>;
    using WriteString = DFHack::Function<module, write_string,
                                         dfproto::RemoteMemAccess::WriteIn,
                                         dfproto::EmptyMessage>;
};


class DFInstanceDFHack : public DFInstance {
    Q_OBJECT
public:
    DFInstanceDFHack(QObject *parent=0);
    virtual ~DFInstanceDFHack();
    void find_running_copy();

    USIZE read_raw(const VIRTADDR addr, const USIZE bytes, void *buffer);
    QString read_string(const VIRTADDR addr);

    // Writing
    USIZE write_raw(const VIRTADDR addr, const USIZE bytes, const void *buffer);
    USIZE write_string(const VIRTADDR addr, const QString &str);

    bool df_running();

    bool attach();
    bool detach();

protected:
    bool set_pid() {return false;}

private:
    void log_text(DFHack::Color color, const QString &text) const;
    void connection_changed(bool connected);
    void connection_error(QAbstractSocket::SocketError error, const QString &error_string);

    bool m_connected;
    DFHack::Client m_dfhack_client;
    DFHack::Core::Suspend m_core_suspend;
    DFHack::Core::Resume m_core_resume;
    RemoteMemAccess::Info m_remote_info;
    RemoteMemAccess::ReadRaw m_remote_read_raw;
    RemoteMemAccess::ReadString m_remote_read_string;
    RemoteMemAccess::WriteRaw m_remote_write_raw;
    RemoteMemAccess::WriteString m_remote_write_string;

    static constexpr std::size_t PageSize = 4096;
    std::map<VIRTADDR, std::array<char, PageSize>> m_page_cache;
    const std::array<char, PageSize> &get_page(VIRTADDR page_addr);

    int m_read_raw_count;
    int m_read_string_count;
    int m_page_download_count;
};

#endif // DFINSTANCE_LINUX_H
