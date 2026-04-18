#include "quantify.h"
#include "WConfig/wconfigdocument.h"
using namespace we::Consts;
using namespace Quantify::Consts;
///
/// \brief Quantify::resolvePath
/// \param doc
/// \param relativePath
/// \return
///
QString Quantify::resolvePath(WConfigDocument* doc, const QString& relativePath) {
    if (!doc) return relativePath;
    QString configDir = doc->get(DirRoot).toString();
    if (configDir.isEmpty()) return relativePath;
    QDir baseDir(configDir);
    return baseDir.filePath(relativePath);
}
///
/// \brief Quantify::resolvePathWithKey
/// \param doc
/// \param key
/// \return
///
QString Quantify::resolvePathWithKey(WConfigDocument *doc, const QString &key)
{
    return resolvePath(doc,doc->get(key).toString());
}

QString Quantify::getConfigDir(WConfigDocument *doc)
{
    return doc->get(DirRoot).toString();
}
