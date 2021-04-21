#include <iostream>
#include <cstdio>

#include <QtCore>
#include <QFile>
#include <QMap>

#include "coor.hxx"

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("Datev2Coor");
    app.setOrganizationName("keenlogics gmbh");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Datev2Coor");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inputFileOption({ "i", "input-file" }, "Specify the file path to the DATEV input file", "path");
    parser.addOption(inputFileOption);

    QCommandLineOption outputFileOption({ "o", "output-file" }, "Specify the file path to the COOR output file", "path");
    parser.addOption(outputFileOption);

    parser.process(app);

    if (!parser.isSet(inputFileOption)) {
        std::cout << "Missing input file path" << std::endl;
        return 1;
    } else if (!parser.isSet(outputFileOption)) {
        std::cout << "Missing output file path" << std::endl;
        return 1;
    }

    const QString inputFilePath = parser.value(inputFileOption);
    const QString outputFilePath = parser.value(outputFileOption);

    QFile inputFile(inputFilePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cout << QStringLiteral("Error opening input file '%0': %1").arg(inputFilePath, inputFile.errorString()).toStdString() << std::endl;
        return 1;
    }

    auto coorData = std::make_unique<coordata>();
    coorData->mandant(QStringLiteral("coor").toStdWString());
    coorData->name(QStringLiteral("Kontakte aus Datev").toStdWString());
    coorData->version(QStringLiteral("3.3").toStdWString());
    coorData->external(QStringLiteral("CRM").toStdWString());
    coordata::bkm_contact_sequence contacts;

    QTextStream in(&inputFile);
    bool firstline = true;

    const auto konto = QStringLiteral("Konto");
    const auto nameFirma = QStringLiteral("Name (Adressattyp Unternehmen)");
    const auto namePerson = QStringLiteral("Name (Adressattyp natürl. Person)");
    const auto nameNone = QStringLiteral("Name (Adressattyp keine Angabe)");
    const auto nameKurz = QStringLiteral("Kurzbezeichnung");
    const auto strasse = QStringLiteral("Straße");
    const auto plz = QStringLiteral("Postleitzahl");
    const auto ort = QStringLiteral("Ort");
    const auto land = QStringLiteral("Land");
    const auto tel = QStringLiteral("Telefon");
    const auto email = QStringLiteral("E-Mail");
    const auto sonst = QStringLiteral("Sonstige");
    const auto ustid = QStringLiteral("EU-UStID");
    const auto anrede = QStringLiteral("Anrede");
    const auto fax = QStringLiteral("Fax");
    const auto internet = QStringLiteral("Internet");
    const auto suchname = QStringLiteral("Alternativer Suchname");

    QMap<QString, int> columns;

    while (!in.atEnd()) {
        QStringList line = in.readLine().split(";");
        for (int i = 0; i < line.size(); i++) {
            if (line.at(i).startsWith("\"")) {
                line[i] = line[i].mid(1);
            }
            if (line.at(i).endsWith("\"")) {
                line[i].chop(1);
            }
        }

        if (firstline) {
            columns[konto] = line.indexOf(konto);
            columns[nameFirma] = line.indexOf(nameFirma);
            columns[namePerson] = line.indexOf(namePerson);
            columns[nameNone] = line.indexOf(nameNone);
            columns[nameKurz] = line.indexOf(nameKurz);
            columns[strasse] = line.indexOf(strasse);
            columns[plz] = line.indexOf(plz);
            columns[ort] = line.indexOf(ort);
            columns[land] = line.indexOf(land);
            columns[tel] = line.indexOf(tel);
            columns[email] = line.indexOf(email);
            columns[sonst] = line.indexOf(sonst);
            columns[ustid] = line.indexOf(ustid);
            columns[anrede] = line.indexOf(anrede);
            columns[fax] = line.indexOf(fax);
            columns[internet] = line.indexOf(internet);
            columns[suchname] = line.indexOf(suchname);

            for (const auto key : columns.keys()) {
                if (columns[key] < 0) {
                    std::cout << "Column not found in input file: " << key.toStdString() << std::endl;;
                    return 1;
                }
            }
            firstline = false;
        } else {
            bkm_contact contact(line[columns[konto]].toStdWString());
            contact.controlcode(QStringLiteral("ui").toStdWString());
            contact.keyid(line[columns[konto]].toStdWString());
            contact.matchfield(QStringLiteral("keyid").toStdWString());
            contact.externalkey(line[columns[konto]].toStdWString());
            contact.synonym(line[columns[nameKurz]].toStdWString());
            contact.name1(line[columns[nameFirma]].toStdWString());
            contact.name2(line[columns[namePerson]].toStdWString());
            contact.name3(line[columns[nameNone]].toStdWString());
            contact.city(line[columns[ort]].toStdWString());
            contact.zipcode(line[columns[plz]].toStdWString());
            contact.street(line[columns[strasse]].toStdWString());
            contact.statecode(line[columns[land]].toStdWString());
            contact.phone(line[columns[tel]].toStdWString());
            contact.phone2(line[columns[sonst]].toStdWString());
            contact.fax(line[columns[fax]].toStdWString());
            contact.email(line[columns[email]].toStdWString());
            contact.vatid(line[columns[ustid]].toStdWString());
            contact.homepage(line[columns[internet]].toStdWString());
            contacts.push_back(contact);
        }
    }
    coorData->bkm_contact(contacts);

    std::ostringstream stream;
    coordata_(stream, *coorData);

    const QString result = QString::fromStdString(stream.str());
    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        std::cout << QStringLiteral("Error opening output file '%0': %1").arg(outputFilePath, outputFile.errorString()).toStdString() << std::endl;
        return 1;
    }
    if (outputFile.write(result.toUtf8()) < 0) {
        std::cout << "Unable to write result to output file: " << outputFile.errorString().toStdString() << std::endl;
    }
    outputFile.close();

    return 0;
}