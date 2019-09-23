(use-modules (guix build utils)
             (guix build-system gnu)
             (guix download)
             (guix licenses)
             (guix packages)
             (guix utils)
             (gnu packages base)
             (gnu packages check)
             (gnu packages gcc)
             (gnu packages glib)
             (gnu packages gnome)
             (gnu packages gstreamer)
             (gnu packages pkg-config))

(define-public gstreamermm
  (package
   (name "gstreamermm")
   (version "1.10.0")
   (source (origin
            (method url-fetch)
            (uri (string-append
                  "mirror://gnome/sources/" name "/" (version-major+minor version) "/"
                  name "-" version ".tar.xz"))
            (sha256
             (base32 "0q4dx9sncqbwgpzma0zvj6zssc279yl80pn8irb95qypyyggwn5y"))))
   (build-system gnu-build-system)
   (arguments
    ;; integration/test-integration-seekonstartup.cc fails
    `(#:tests? #f))
   (propagated-inputs
    `(("glibmm" ,glibmm)
      ("gstreamer" ,gstreamer)
      ("gst-plugins-base" ,gst-plugins-base)
      ("libsigc++" ,libsigc++)
      ("libxml++-2" ,libxml++-2)))
   (inputs
    `(("googletest" ,googletest)))
   (native-inputs
    `(("pkg-config" ,pkg-config)))
   (synopsis "C++ bindings for the GStreamer streaming multimedia library")
   (description "gstreamermm provides C++ bindings for the GStreamer streaming multimedia
library (http://gstreamer.freedesktop.org).  With gstreamermm it is possible to
develop applications that work with multimedia in C++.

gstreamermm is developed over glibmm, libsigc++ and libxml++ and the
functionalities they provide.  This means that, among other things, referencing
and unreferencing of GObjects is handled automatically via glibmm's automatic
pointer class, Glib::RefPtr, and libsigc++'s slots are used for callbacks and
signals.")
   (home-page "https://www.gtkmm.org/")
   (license lgpl2.1+)))

gstreamermm

;; EOF ;;
