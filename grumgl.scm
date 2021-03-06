;; grumgl - Grumbel's Random Collection of OpenGL Experiments
;; Copyright (C) 2019 Ingo Ruhnke <grumbel@gmail.com>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(use-modules (ice-9 popen)
             (ice-9 rdelim)
             (guix build utils)
             (guix build-system cmake)
             (guix gexp)
             (guix git-download)
             (guix licenses)
             (guix packages)
             (gnu packages base)
             (gnu packages boost)
             (gnu packages gcc)
             (gnu packages gl)
             (gnu packages gtk)
             (gnu packages linux)
             (gnu packages maths)
             (gnu packages pkg-config)
             (gnu packages python)
             (gnu packages sdl)
             )

(define %source-dir (dirname (current-filename)))

(define current-commit
  (with-directory-excursion %source-dir
                            (let* ((port   (open-input-pipe "git describe --tags"))
                                   (output (read-line port)))
                              (close-pipe port)
                              (string-trim-right output #\newline))))

(define (source-predicate . dirs)
  (let ((preds (map (lambda (p)
                      (git-predicate (string-append %source-dir p)))
                    dirs)))
    (lambda (file stat)
      (let loop ((f (car preds))
                 (rest (cdr preds)))
        (if (f file stat)
            #t
            (if (not (nil? rest))
                (loop (car rest) (cdr rest))
                #f))))))

(define gstreamermm (load "gstreamermm.scm"))

(define-public grumgl
  (package
   (name "grumgl")
   (version current-commit)
   (source (local-file %source-dir
                       #:recursive? #t
                       #:select? (source-predicate ""
                                                   "/external/WiiC"
                                                   "/external/wiic-2013-02-12"
                                                   "/external/benchmark"
                                                   "/external/googletest")))
   (arguments
    `(#:tests? #f
      #:configure-flags '()
      #:phases (modify-phases
                %standard-phases
                (add-before 'configure 'fixgcc9
                            (lambda _
                              (unsetenv "C_INCLUDE_PATH")
                              (unsetenv "CPLUS_INCLUDE_PATH")))
                )))
   (build-system cmake-build-system)
   (inputs
    `(("sdl2" ,sdl2)
      ("sdl2-image" ,sdl2-image)
      ("mesa" ,mesa)
      ("cairomm" ,cairomm)
      ("glew" ,glew)
      ("boost" ,boost)
      ("gstreamermm" ,gstreamermm)
      ("bluez" ,bluez)
      ("glm" ,glm)))
   (native-inputs
    `(("pkg-config" ,pkg-config)
      ("gcc" ,gcc-9)))
   (propagated-inputs
    `(("python" ,python)))
   (synopsis "Grumbel's Random Collection of OpenGL Experiments")
   (description "This repository is a messy collection of OpenGL programming
experiments. It contains a Blender export script for 3D models along
with a viewer and a tiny 3D engine. It also contains some support for
the Cybermaxx VR headset as well as anaglyph 3D glasses.")
   (home-page "https://gitlab.com/grumbel/grumgl")
   (license gpl3+)))

grumgl

;; EOF ;;
