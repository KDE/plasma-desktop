The internallibkonq/ folder contains a subset of the libkonq
library originally from kde-baseapps.git.

This was added here temporarily to avoid a dependency of
plasma-desktop on kde-baseapps.

The long-term plan is to split up libkonq and distribute its
useful parts over various parts of KDE Frameworks 5, and then
eliminate this duplication.

DO NOT MODIFY this copy of the library to avoid divergence
from the upstream source. Any patches need to be applied to
kde-baseapps first.

Signed of by: David Faure <faure@kde.org>, library maintainer
              Kevin Ottens <ervin@kde.org>, KF5 coordinator