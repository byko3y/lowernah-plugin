#include <QDir>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QSet>
#include <QtPlugin>
#include <QWidget>

#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "psiplugin.h"
#include "stanzafilter.h"

static QString jidBare(const QString &jid)
{
	const int slash = jid.indexOf(QLatin1Char('/'));
	return (slash < 0 ? jid : jid.left(slash)).toLower();
}

static QString jidResource(const QString &jid)
{
	const int slash = jid.indexOf(QLatin1Char('/'));
	return slash < 0 ? QString() : jid.mid(slash + 1);
}

static void lowerTextNodes(QDomNode node)
{
	for (QDomNode c = node.firstChild(); !c.isNull(); c = c.nextSibling()) {
		if (c.isText() || c.isCDATASection())
			c.setNodeValue(c.nodeValue().toLower());
		else if (c.isElement())
			lowerTextNodes(c);
	}
}

class LowerNahPlugin : public QObject,
                       public PsiPlugin,
                       public StanzaFilter,
                       public ApplicationInfoAccessor
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "me.bykozy.lowernah")
	Q_INTERFACES(PsiPlugin StanzaFilter ApplicationInfoAccessor)

public:
	LowerNahPlugin()
		: enabled_(false)
		, appInfo_(nullptr)
		, watcher_(nullptr)
	{
	}

	QString name() const override { return QStringLiteral("LowerNah"); }
	QString shortName() const override { return QStringLiteral("lowernah"); }
	QString version() const override { return QStringLiteral("0.1"); }
	QPixmap icon() const override { return QPixmap(); }

	QWidget *options() override
	{
		auto *label = new QLabel(tr(
			"Edit per-room lists in:\n%1\n\n"
			"Each file is named after the MUC room JID "
			"(for example room@conference.example.com).\n"
			"One nickname or JID per line. Lines starting with # are comments.")
					     .arg(configDir()));
		label->setWordWrap(true);
		label->setMargin(8);
		return label;
	}

	bool enable() override
	{
		if (!appInfo_)
			return false;

		if (!watcher_) {
			watcher_ = new QFileSystemWatcher(this);
			connect(watcher_, &QFileSystemWatcher::fileChanged,
				this, &LowerNahPlugin::onFileChanged);
			connect(watcher_, &QFileSystemWatcher::directoryChanged,
				this, &LowerNahPlugin::onDirectoryChanged);
		}

		reloadAll();
		enabled_ = true;
		return true;
	}

	bool disable() override
	{
		enabled_ = false;
		roomNicks_.clear();
		if (watcher_) {
			const QStringList watched = watcher_->files() + watcher_->directories();
			if (!watched.isEmpty())
				watcher_->removePaths(watched);
		}
		return true;
	}

	void applyOptions() override {}
	void restoreOptions() override {}

	bool incomingStanza(int account, const QDomElement &xml) override
	{
		Q_UNUSED(account);
		if (!enabled_ || xml.tagName() != QLatin1String("message"))
			return false;

		const QString from = xml.attribute(QStringLiteral("from"));
		if (from.isEmpty())
			return false;

		const QString room = jidBare(from);
		const QString nick = jidResource(from).toLower();
		if (nick.isEmpty() || !roomNicks_.contains(room))
			return false;

		const QSet<QString> &ids = roomNicks_.value(room);
		const QString full = from.toLower();
		if (!ids.contains(nick) && !ids.contains(full))
			return false;

		lowerTextNodes(xml.firstChildElement(QStringLiteral("body")));
		lowerTextNodes(xml.firstChildElement(QStringLiteral("html")));
		return false;
	}

	bool outgoingStanza(int account, QDomElement &xml) override
	{
		Q_UNUSED(account);
		Q_UNUSED(xml);
		return false;
	}

	void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost *host) override
	{
		appInfo_ = host;
	}

private:
	QString configDir() const
	{
		if (!appInfo_)
			return QString();
		return appInfo_->appHomeDir(ApplicationInfoAccessingHost::ConfigLocation)
			+ QLatin1String("/lowernah");
	}

	void reloadAll()
	{
		roomNicks_.clear();

		const QString dirPath = configDir();
		if (dirPath.isEmpty())
			return;

		QDir dir(dirPath);
		if (!dir.exists())
			dir.mkpath(QStringLiteral("."));

		if (watcher_) {
			const QStringList watched = watcher_->files() + watcher_->directories();
			if (!watched.isEmpty())
				watcher_->removePaths(watched);
			watcher_->addPath(dir.absolutePath());
		}

		const QStringList files = dir.entryList(QDir::Files | QDir::Readable | QDir::Hidden, QDir::Name);
		for (const QString &name : files) {
			if (name.startsWith(QLatin1Char('.')))
				continue;
			loadRoomFile(dir.absoluteFilePath(name));
			if (watcher_)
				watcher_->addPath(dir.absoluteFilePath(name));
		}
	}

	void loadRoomFile(const QString &path)
	{
		const QString fileName = QFileInfo(path).fileName();
		QString room = fileName;
		if (room.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive))
			room.chop(4);
		room = room.toLower();
		if (!room.contains(QLatin1Char('@')))
			return;

		QSet<QString> ids;
		QFile f(path);
		if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
			while (!f.atEnd()) {
				const QString line = QString::fromUtf8(f.readLine()).trimmed();
				if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
					continue;
				ids.insert(line.toLower());
			}
		}
		roomNicks_.insert(room, ids);
	}

	void onFileChanged(const QString &path)
	{
		loadRoomFile(path);
		if (watcher_ && QFile::exists(path) && !watcher_->files().contains(path))
			watcher_->addPath(path);
	}

	void onDirectoryChanged(const QString &)
	{
		reloadAll();
	}

	bool enabled_;
	ApplicationInfoAccessingHost *appInfo_;
	QFileSystemWatcher *watcher_;
	QHash<QString, QSet<QString>> roomNicks_;
};

#include "lowernah.moc"
