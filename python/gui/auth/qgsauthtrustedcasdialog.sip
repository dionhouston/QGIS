class QgsAuthTrustedCAsDialog : QDialog
{
%TypeHeaderCode
#include <qgsauthtrustedcasdialog.h>
%End

  public:
    explicit QgsAuthTrustedCAsDialog( QWidget *parent /TransferThis/ = 0,
                                      QList<QSslCertificate> trustedCAs = QList<QSslCertificate>() );
    ~QgsAuthTrustedCAsDialog();
};
