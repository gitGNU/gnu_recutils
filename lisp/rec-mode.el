;;; rec-mode.el --- Major mode for viewing/editing rec files

;; Copyright (C) 2009 Jose E. Marchesi

;; Maintainer: Jose E. Marchesi

;; This file is NOT part of GNU Emacs.

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;;; Commentary:

;; A major mode for editing rec files.

;;; Code:

;; Customization

(defgroup rec-mode nil
  "rec-mode subsystem"
  :group 'applications
  :link '(url-link "http://www.gnu.org/software/rec"))

;; Variables and constants that the user does not want to touch
;; (really!)

(defconst rec-keyword-rec "%rec"
  ;; Remember to update `rec-font-lock-keywords' if you change this
  ;; value!!
  "Rec keyword.")

(defvar rec-comment-re "^#.*\n?"
  "regexp denoting a comment line")

(defvar rec-field-name-re
  "^\\([a-zA-Z0-1_%-]+:\\)+"
  "Regexp matching a field name")

(defvar rec-field-value-re
  (let ((ret-re "\n\\+ ?")
        (esc-ret-re "\\\\\n"))
    (concat
     "\\("
     "\\(" ret-re "\\)*"
     "\\(" esc-ret-re "\\)*"
     "[^\\\n]*"
     "\\)*"))
  "Regexp matching a field value")

(defvar rec-field-re
  (concat rec-field-name-re
          rec-field-value-re
          "\n")
  "Regexp matching a field")

(defvar rec-record-re
  (concat rec-field-re "\\(" rec-field-re "\\|" rec-comment-re "\\)*")
  "Regexp matching a record")

(defvar rec-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?# "<" st)   ; Comment start
    (modify-syntax-entry ?\n ">" st)  ; Comment end
    st)
  "Syntax table used in rec-mode")

(defvar rec-font-lock-keywords
  `(("^%\\(rec\\|key\\|unique\\|mandatory\\|doc\\):" . font-lock-keyword-face)
    (,rec-field-name-re . font-lock-variable-name-face)
    ("^\\+" . font-lock-constant-face))
  "Font lock keywords used in rec-mode")

(defvar rec-mode-edit-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-c\C-c" 'rec-finish-editing)
    map)
  "Keymap for rec-mode")

(defvar rec-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "n" 'rec-cmd-goto-next-rec)
    (define-key map "p" 'rec-cmd-goto-previous-rec)
    (define-key map "\C-ce" 'rec-edit-field)
    (define-key map "e" 'rec-edit-record)
    (define-key map "E" 'rec-edit-type)
    (define-key map "B" 'rec-edit-buffer)
    (define-key map "t" 'rec-cmd-show-descriptor)
    (define-key map "\C-ct" 'rec-find-type)
    (define-key map "j" 'rec-cmd-jump)
    (define-key map "b" 'rec-cmd-jump-back)
    map)
  "Keymap for rec-mode")

;; Parsing functions (rec-parse-*)
;;
;; Those functions read the contents of the buffer (starting at the
;; current position of the pointer) and try to parse field, comment
;; and records structures.

(defun rec-parse-comment ()
  "Parse and return a comment starting at point.

Return a list whose first element is the symbol 'comment and the
second element is the string with the contents of the comment,
including the leading #:

   (comment \"# foo\")

If the point is not at the beginning of a comment return nil"
  (when (and (equal (current-column) 0)
             (looking-at rec-comment-re))
    (goto-char (match-end 0))
    (list 'comment (buffer-substring-no-properties (match-beginning 0)
                                                   (match-end 0)))))

(defun rec-parse-field-name ()
  "Parse and return a field name starting at point.

Return a list with whose elements are the parts of the field
name.  For the name a:b:c:d: the following list is returned:

   ('a' 'b' 'c' 'd')

If the point is not at the beginning of a field name return nil"
  (when (and (equal (current-column) 0)
             (looking-at rec-field-name-re))
    (goto-char (match-end 0))
    (split-string
     (buffer-substring-no-properties (match-beginning 0)
                                     (- (match-end 0) 1))
     ":")))

(defun rec-parse-field-value ()
  "Return the field value under the pointer.

Return a string containing the value of the field.

If the pointer is not at the beginning of a field value, return
nil"
  (when (looking-at rec-field-value-re)
    (goto-char (match-end 0))
    (let ((val (buffer-substring-no-properties (match-beginning 0)
                                               (match-end 0))))
      ;; Replace escaped newlines
      (setq val (replace-regexp-in-string "\\\\\n" "" val))
      ;; Replace continuation lines
      (setq val (replace-regexp-in-string "\n\\+ ?" "\n" val))
      ;; Trim blanks in the value
      (setq val (replace-regexp-in-string "^[ \t]+" "" val))
      (setq val (replace-regexp-in-string "[ \t]+$" "" val))
      val)))

(defun rec-parse-field ()
  "Return a structure describing the field starting from the
pointer.

The returned structure is a list whose first element is the
symbol 'field', the second element is the name of the field and
the second element is the value of the field:

   (field FIELD-NAME FIELD-VALUE)

If the pointer is not at the beginning of a field
descriptor then return nil"
  (let (field-name field-value)
    (and (setq field-name (rec-parse-field-name))
         (setq field-value (rec-parse-field-value)))
    (when (and field-name field-value)
        (list 'field field-name field-value))))

(defun rec-parse-record ()
  "Return a structure describing the record starting from the pointer.

The returned structure is a list of fields preceded by the symbol
'record':

   (record FIELD-1 FIELD-2 ... FIELD-N)

If the pointer is not at the beginning of a record, then return
nil"
  (let (record field-or-comment)
    (while (setq field-or-comment (or (rec-parse-field)
                                      (rec-parse-comment)))
      (setq record (cons field-or-comment record))
      ;; Skip the newline finishing the field or the comment
      (when (looking-at "\n") (goto-char (match-end 0))))
    (setq record (cons 'record (reverse record)))))

;; Writer functions (rec-insert-*)
;;
;; Those functions dump the written representation of the parser
;; structures (field, comment, record, etc) into the current buffer
;; starting at the current position.

(defun rec-insert-comment (comment)
  "Insert the written form of COMMENT in the current buffer"
  (when (and (listp comment) 
             (equal (car comment) 'comment))
    (insert (cadr comment) "\n")))

(defun rec-insert-field-name (field-name)
  "Insert the written form of FIELD-NAME in the current buffer"
  (when (listp field-name)
    (mapcar (lambda (elem)
              (when (stringp elem) (insert elem ":")))
            field-name)))

(defun rec-insert-field-value (field-value)
  "Insert the written form of FIELD-VALUE in the current buffer"
  (when (stringp field-value)
    (let ((val field-value))
      ;; FIXME: Maximum line size
      (insert (replace-regexp-in-string "\n" "\n+ " val)))
    (insert "\n")))

(defun rec-insert-field (field)
  "Insert the written form of FIELD in the current buffer"
  (when (and (listp field)
             (equal (car field) 'field))
    (when (rec-insert-field-name (cadr field))
      (insert " ")
      (rec-insert-field-value (nth 2 field)))))

(defun rec-insert-record (record)
  "Insert the written form of RECORD in the current buffer"
  (when (and (listp record)
             (equal (car record) 'record))
    (rec-insert-record-2 (cdr record))))

(defun rec-insert-record-2 (record)
  "Insert the written form of RECORD in the current buffer.
Recursive part"
  (when (and record (listp record))
    (let ((elem (car record)))
      (cond
       ((equal (car elem) 'comment)
        (rec-insert-comment elem))
       ((equal (car elem) 'field)
        (rec-insert-field elem))))
    (rec-insert-record-2 (cdr record))))

;; Operations on record structures
;;
;; Those functions retrieve or set properties of field structures.

(defun rec-record-assoc (name record)
  "Get a list with the values of the fields in RECORD named NAME.  If no such
field exists in RECORD then nil is returned"
  (when (and (listp record)
             (equal (car record) 'record))
    (let (result)
      (mapcar (lambda (field)
                (when (and (equal (car field) 'field)
                           (equal name (cadr field)))
                  (setq result (cons (nth 2 field) result))))
              (cdr record))
      (reverse result))))

(defun rec-record-names (record)
  "Get a list of the field names in the record"
  (when (and (listp record)
             (equal (car record) 'record))
    (let (result)
      (mapcar (lambda (field)
                (when (and (equal (car field) 'field))
                  (setq result (cons (nth 1 field) result))))
              (cdr record))
      (reverse result))))

;; Operations on field structures
;;
;; Those functions retrieve or set properties of field structures.

(defun rec-field-p (field)
  "Determine if the provided structure is a field"
  (and (listp field)
       (= (length field) 3)
       (equal (car field) 'field)))

(defun rec-field-name (field)
  "Return the name of the provided field"
  (when (rec-field-p field)
    (cadr field)))

(defun rec-field-value (field)
  "Return the value of the provided field"
  (when (rec-field-p field)
    (nth 2 field)))

;; Get entities under pointer
;;
;; Those functions retrieve structures of the entities under pointer
;; like comments, fields and records.  If the especified entity is not
;; under the pointer then nil is returned.

(defun rec-beginning-of-field-pos ()
  "Return the position of the beginning of the current field, or
nil if the pointer is not on a field."
  (save-excursion
    (beginning-of-line)
    (cond
     ((and (not (= (line-beginning-position) 1))
           (or (looking-at "+")
               (looking-back "\\\\\n" 2)))
      (forward-line -1)
      (rec-beginning-of-field))
     ((looking-at rec-field-name-re)
      (point))
     (t
      nil))))

(defun rec-end-of-field-pos ()
  "Return the position of the end of the current field, or nil if
the pointer is not on a field."
  (let ((begin-pos (rec-beginning-of-field-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        (when (looking-at rec-field-re)
          (match-end 0))))))

(defun rec-beginning-of-comment-pos ()
  "Return the position of the beginning of the current comment,
or nil if the pointer is not on a comment."
  (save-excursion
    (beginning-of-line)
    (when (looking-at rec-comment-re)
      (point))))

(defun rec-end-of-comment-pos ()
  "Return the position of the end of the current comment,
or nil if the pointer is not on a comment."
  (let ((begin-pos (rec-beginning-of-comment-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        (when (looking-at rec-comment-re)
          (match-end 0))))))

(defun rec-beginning-of-record-pos ()
  "Return the position of the beginning of the current record, or nil if
the pointer is not on a record."
  (save-excursion
    (let (field-pos)
      (while (and (not (equal (point) (point-min)))
                  (or (setq field-pos (rec-beginning-of-field-pos))
                      (setq field-pos (rec-beginning-of-comment-pos))))
        (goto-char field-pos)
        (if (not (equal (point) (point-min)))
            (backward-char)))
      (if (not (equal (point) (point-min)))
          (forward-char))
      (when (looking-at rec-record-re)
        (point)))))

(defun rec-end-of-record-pos ()
  "Return the position of the end of the current record,
or nil if the pointer is not on a record."
  (let ((begin-pos (rec-beginning-of-record-pos)))
    (when begin-pos
      (save-excursion
        (goto-char begin-pos)
        (when (looking-at rec-record-re)
          (match-end 0))))))

(defun rec-current-field ()
  "Return a structure with the contents of the current field.
The current field is the field where the pointer is."
  (save-excursion
    (let ((begin-pos (rec-beginning-of-field-pos)))
      (when begin-pos
        (goto-char begin-pos)
        (rec-parse-field)))))

(defun rec-current-record ()
  "Return a structure with the contents of the current record.
The current record is the record where the pointer is"
  (save-excursion
    (let ((begin-pos (rec-beginning-of-record-pos)))
      (when begin-pos
        (goto-char begin-pos)
        (rec-parse-record)))))

;; Visibility
;;
;; These functions manage the visibility in the rec buffer.

(defun rec-narrow-to-record ()
  "Narrow to the current record, if any"
  (let ((begin-pos (rec-beginning-of-record-pos))
        (end-pos (rec-end-of-record-pos)))
    (if (and begin-pos end-pos)
        (narrow-to-region begin-pos end-pos))))

(defun rec-narrow-to-type (type)
  "Narrow to the specified type, if any"
  (let ((begin-pos (or (rec-type-pos type) (point-min)))
        (end-pos (or (rec-type-pos (rec-type-next type)) (point-max))))
    (narrow-to-region begin-pos end-pos)))

;; Record collection management
;;
;; These functions perform the management of the collection of records
;; in the buffer.
    
(defun rec-update-buffer-descriptors ()
  "Get a list of the record descriptors in the current buffer."
  (setq rec-buffer-descriptors
        (save-excursion
          (let (records rec marker)
            (goto-char (point-min))
            (while (and (not (= (point) (point-max)))
                        (re-search-forward rec-record-re nil t))
              (goto-char (match-beginning 0))
              (setq marker (point-marker))
              (setq rec (rec-parse-record))
              (when (rec-record-assoc (list rec-keyword-rec) rec)
                (setq records (cons (list 'descriptor rec marker) records)))
              (if (not (= (point) (point-max)))
                  (forward-char)))
            (reverse records)))))

(defun rec-buffer-types ()
  "Return a list with the names of the record types in the
existing buffer."
  ;; If a descriptor has more than a %rec field, then the first one is
  ;; used.  The rest are ignored.
  (mapcar

   (lambda (elem) (car elem))
   (mapcar
    (lambda (elem)
      (rec-record-assoc (list rec-keyword-rec) (cadr elem)))
    rec-buffer-descriptors)))

(defun rec-type-p (type)
  "Determine if there are records of type TYPE in the current
file."
  (member type (rec-buffer-types)))

(defun rec-goto-type (type)
  "Goto the beginning of the descriptor with type TYPE.

If the type do not exist in the current buffer then
this function returns nil."
  (if (not type)
      ;; If there is a regular record in the 
      ;; beginning of the file, go there.
      (if (save-excursion
            (goto-char (point-min))
            (rec-goto-next-rec)
            (rec-regular-p))
          (progn
            (goto-char (point-min))
            (rec-goto-next-rec)
            t)
        nil)
    (let (found
          (descriptors rec-buffer-descriptors))
      (mapcar
       (lambda (elem)
         (when (equal (car (rec-record-assoc (list rec-keyword-rec)
                                             (cadr elem)))
                      type)
           (setq found t)
           (goto-char (nth 2 elem))))
       descriptors)
      found)))

(defun rec-type-pos (type)
  "Return the position where the records of type TYPE start in
the current file.  If no records of type TYPE are defined in the
current file then return nil."
  (when (rec-type-p type)
    (save-excursion
      (rec-goto-type type)
      (point))))

(defun rec-type-next (type)
  "Return the name of the type following TYPE in the file, if
any.  If the specified type is the last appearing in the file,
or the specified type does not exist, then return nil."
  (let ((types (member type (rec-buffer-types))))
    (nth 1 types)))

(defun rec-type-previous (type)
  "Return the name of the type preceding TYPE in the file, if
any.  If the specified type is the first appearing in the file,
or the specified type does not exist, then return nil."
  (let ((types (member type (reverse (rec-buffer-types)))))
    (nth 1 types)))

(defun rec-goto-next-rec ()
  "Move the pointer to the beginning of the next record in the
file."
  (let ((pos (save-excursion
               (rec-end-of-record)
               (when (re-search-forward rec-record-re nil t)
                 (match-beginning 0)))))
    (when pos 
        (goto-char pos)
        t)))

(defun rec-goto-previous-rec ()
  "Move the pointer to the end of the previous record in the
file."
    (let ((pos (save-excursion
                 (rec-beginning-of-record)
                 (if (not (= (point) (point-min)))
                     (backward-char))
                 (when (and (re-search-backward rec-record-re nil t)
                            (rec-beginning-of-record))
                   (point)))))
      (when pos
        (goto-char pos)
        t)))

(defun rec-type-first-rec-pos (type)
  "Return the position of the first record of the specified TYPE.

If TYPE is nil then return the position of the first regular record in the file.
If there are no regular records in the file, return nil."
  (save-excursion
    (when (or (not type) (rec-type-p type))
      (if type
          (rec-goto-type type)
        (goto-char (point-min)))
      ;; Find the next regular record
      (when (and (rec-goto-next-rec)
                 (rec-regular-p))
        (point)))))

(defun rec-goto-type-first-rec (type)
  "Goto to the first record of type TYPE present in the file.
If TYPE is nil then goto to the first Unknown record on the file.

If the record is found, return its position.
If no such record exist then don't move and return nil."
  (let ((pos (rec-type-first-rec-pos type)))
    (when pos
      (goto-char pos))))

(defun rec-count (&optional type)
  "If TYPE is a string, return the number of records of the
specified type in the current file.

If TYPE is nil, return the number of records in the current file
not including the record descriptors.

If TYPE is t, return the number of records in the current file,
including the record descriptors.

XXX: to test after rec-map gets written.
XXX: update doc to take into account the usage of the `&optional'
keyword."
  (length (rec-map
           (lambda () t)
           type)))

(defun rec-map (function type)
  "XXX"
  )

(defun rec-regular-p ()
  "Return t if the record under point is a regular record.
Return nil otherwise."
  (let ((rec (rec-current-record)))
    (when rec
      (= (length (rec-record-assoc (list rec-keyword-rec) rec))
         0))))

(defun rec-record-type ()
  "Return the type of the record under point.

If the record is of no known type, return nil."
  (car (rec-record-assoc (list rec-keyword-rec)
                         (cadr (rec-record-descriptor)))))

(defun rec-record-descriptor ()
  "XXX"
  (when (rec-current-record)
    (let ((descriptors rec-buffer-descriptors)
          descriptor type position found
          (i 0))
      (while (and (not found)
                  (< i (length descriptors)))
        (setq descriptor (nth i rec-buffer-descriptors))
        (setq position (marker-position (nth 2 descriptor)))
        (if (and (> (point) position)
                 (or (= i (- (length rec-buffer-descriptors) 1))
                     (< (point) (marker-position (nth 2 (nth (+ i 1) rec-buffer-descriptors))))))
            (setq found t)
          (setq i (+ i 1))))
      (when found
          descriptor))))
                
;; Searching functions

(defun rec-search-first (type name value)
  "Return the position of the beginning of the record of type TYPE
containing a field NAME:VALUE.

If such a record is not found then return nil."
  (save-excursion
    (let (found end-of-type record)
      (when (rec-goto-type type)
        (while (and (not found) (not end-of-type)
                    (rec-goto-next-rec))
          ;; Read a record
          (setq record (rec-current-record))
          ;; Check if found
          (if (member value (rec-record-assoc name record))
              (setq found t)
            ;; Check end-of-type
            (if (rec-record-assoc (list rec-keyword-rec) record)
                (setq end-of-type t))))
        (when found (point))))))

;; Getting data

(defun rec-sel (what name value &optional type)
  "Not working.
XXX"
  (save-excursion
    (mapcar
     (lambda (type)
       (let ((pos (rec-search-first type name value)))
         (when pos
           (goto-char pos)
           (rec-record-assoc what (rec-current-record)))))
     (rec-buffer-types))))

;; Navigation

(defun rec-show-type (type)
  "Show the records of the given type"
  (widen)
;;  (unless (rec-goto-type-first-rec type)
  (unless (rec-goto-type type)
    (message "No records with that type found in the file"))
  (rec-show-record))

(defun rec-show-record ()
  "Show the record under the point"
  (setq buffer-read-only t)
  (rec-narrow-to-record)
  (rec-set-mode-line (rec-record-type)))

;; Mode line

(defun rec-set-mode-line (str)
  "Set the modeline in rec buffers"
  (setq mode-line-buffer-identification
        (list 20
              "%b " str)))

;; Commands
;;
;; The following functions are implementing commands available in the
;; modes.

(defun rec-edit-field ()
  "Edit the contents of the field under point in a separate
buffer"
  (interactive)
  (let* (edit-buf
         (field (rec-current-field))
         (field-value (rec-field-value field))
         (field-name (rec-field-name field))
         (pointer (rec-beginning-of-field-pos))
         (prev-buffer (current-buffer)))
    (if field-value
        (progn
          (setq edit-buf (get-buffer-create "Rec Edit"))
          (set-buffer edit-buf)
          (delete-region (point-min) (point-max))
          (rec-edit-field-mode)
          (make-local-variable 'rec-field-name)
          (setq rec-field-name field-name)
          (make-local-variable 'rec-marker)
          (setq rec-marker (make-marker))
          (set-marker rec-marker pointer prev-buffer)
          (make-local-variable 'rec-buffer)
          (setq rec-prev-buffer prev-buffer)
          (setq rec-pointer pointer)
          (insert field-value)
          (switch-to-buffer-other-window edit-buf)
          (goto-char (point-min))
          (message "Edit the value of the field and press C-cC-c to exit"))
      (message "Not in a field"))))

(defun rec-finish-editing-field ()
  "Stop editing the value of a field."
  (interactive)
  (let ((marker rec-marker)
        (prev-pointer rec-pointer)
        (edit-buffer (current-buffer))
        (name rec-field-name)
        (value (buffer-substring-no-properties (point-min) (point-max))))
    (delete-window)
    (switch-to-buffer rec-prev-buffer)
    (let ((buffer-read-only nil))
      (kill-buffer edit-buffer)
      (goto-char marker)
      (rec-delete-field)
      (rec-insert-field (list 'field
                              name
                              value))
      (goto-char prev-pointer))))
    
(defun rec-beginning-of-field ()
  "Goto to the beginning of the current field"
  (interactive)
  (let ((pos (rec-beginning-of-field-pos)))
    (when pos
      (goto-char pos))))

(defun rec-end-of-field ()
  "Goto to the end of the current field"
  (interactive)
  (let ((pos (rec-end-of-field-pos)))
    (when pos
      (goto-char pos))))

(defun rec-beginning-of-record ()
  "Goto to the beginning of the current record"
  (interactive)
  (let ((pos (rec-beginning-of-record-pos)))
    (when pos
      (goto-char pos))))

(defun rec-end-of-record ()
  "Goto to the end of the current record"
  (interactive)
  (let ((pos (rec-end-of-record-pos)))
    (when pos
      (goto-char pos))))

(defun rec-kill-field ()
  "Kill the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (when (and begin-pos end-pos)
      (kill-region begin-pos end-pos))))

(defun rec-copy-field ()
  "Copy the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (when (and begin-pos end-pos)
      (copy-region-as-kill begin-pos end-pos))))

(defun rec-delete-field ()
  "Delete the current field"
  (interactive)
  (let ((begin-pos (rec-beginning-of-field-pos))
        (end-pos (rec-end-of-field-pos)))
    (when (and begin-pos end-pos)
      (delete-region begin-pos end-pos))))

(defun rec-copy-record ()
  "Copy the current record"
  (interactive))

(defun rec-find-type ()
  "Goto the beginning of the descriptor with a given type."
  (interactive)
  (let ((type (completing-read "Record type: "
                               (save-restriction
                                 (widen)
                                 (rec-buffer-types)))))
    (if (equal type "") (setq type nil))
    (rec-show-type type)))

(defun rec-cmd-goto-next-rec ()
  "Move the pointer to the beginning of the next record in the
file.  Interactive version."
  (interactive)
  (widen)
  (if (save-excursion
        (or (not (rec-goto-next-rec))
            (not (rec-regular-p))))
      (message "No more records")
    (rec-goto-next-rec))
  (rec-show-record))

(defun rec-cmd-goto-previous-rec ()
  "Move the pointer to the beginning of the previous record in
the file.  Interactive version."
  (interactive)
  (widen)
  (if (save-excursion
        (or (not (rec-regular-p))))
      (message "No more records")
    (rec-goto-previous-rec))
  (rec-show-record))
    
(defun rec-cmd-jump ()
  "Jump to the first record containing the reference under
point."
  (interactive)
  (widen)
  (let (size field name value)
    (if (setq field (rec-current-field))
        (progn (setq name (rec-field-name field))
               (setq value (rec-field-value field))
               (if (or (= (length name) 2)
                       (= (length name) 3))
                   (progn
                     (let* ((field-type (nth 0 name))
                            (field-name (nth 1 name))
                            (pos (rec-search-first field-type
                                                   (list field-name)
                                                   value)))
                       (if pos
                           (progn
                             (setq rec-jump-back (point-marker))
                             (goto-char pos)
                             (rec-narrow-to-record))
                         (message "Not found."))))
                 (message "Not in a reference.")
                 (rec-show-record)))
      (message "Not in a reference.")
      (save-excursion
        (rec-goto-previous-rec)
        (rec-show-record)))))

(defun rec-cmd-jump-back ()
  "Undo the previous jump"
  (interactive)
  (if rec-jump-back
      (progn
        (widen)
        (goto-char (marker-position rec-jump-back))
        (rec-show-record)
        (setq rec-jump-back nil))
    (message "No previous position to jump")))
  
(defun rec-edit-record ()
  "Go to the record edition mode"
  (interactive)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (rec-set-mode-line "Edit Record")
  (message "Editing: Press C-c C-c when you are done"))

(defun rec-edit-type ()
  "Go to the type edition mode"
  (interactive)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (widen)
  (rec-narrow-to-type (rec-record-type))
  (setq rec-update-p t)
  (rec-set-mode-line "Edit Type")
  (message "Editing:  Press C-c C-c when you are done"))

(defun rec-edit-buffer ()
  "Go to the buffer edition mode"
  (interactive)
  (setq buffer-read-only nil)
  (use-local-map rec-mode-edit-map)
  (widen)
  (setq rec-update-p t)
  (rec-set-mode-line "Edit Buffer")
  (message "Editing: Press C-c C-c when you are done"))

(defun rec-finish-editing ()
  "Go back from the record edition mode"
  (interactive)
  (use-local-map rec-mode-map)
  (or (rec-current-record)
      (rec-goto-next-rec)
      (rec-goto-previous-rec))
  (when rec-update-p
    (rec-update-buffer-descriptors)
    (setq rec-update-p nil))
  (rec-show-record)
  ;; TODO: Restore modeline
  (message "End of edition"))

(defun rec-cmd-show-descriptor ()
  "Show the descriptor record of the current record.

This jump sets jump-back."
  (interactive)
  (let ((type (rec-record-type)))
    (when type
      (setq rec-jump-back (point-marker))
      (rec-show-type type))))
      
;; Definition of modes
  
(defun rec-mode ()
  "A major mode for editing rec files.

Commands:
\\{rec-mode-map}"
  (interactive)
  (kill-all-local-variables)
  ;; Local variables
  (make-local-variable 'font-lock-defaults)
  (make-local-variable 'rec-type)
  (make-local-variable 'rec-buffer-descriptors)
  (make-local-variable 'rec-jump-back)
  (make-local-variable 'rec-update-p)
  (setq rec-jump-back nil)
  (setq rec-update-p nil)
  (setq font-lock-defaults '(rec-font-lock-keywords))
  (use-local-map rec-mode-map)
  (set-syntax-table rec-mode-syntax-table)
  (setq mode-name "Rec")
  (setq major-mode 'rec-mode)
  (setq buffer-read-only t)
  ;; Goto the first record of the first type (including the Unknown)
  (rec-update-buffer-descriptors)
  (setq rec-type (car (rec-buffer-types)))
  (rec-show-type rec-type))

(defvar rec-edit-field-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-c\C-c" 'rec-finish-editing-field)
    map)
  "Keymap for rec-edit-field-mode")

(defun rec-edit-field-mode ()
  "A major mode for editing rec field values.

Commands:
\\{rec-edit-field-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (use-local-map rec-edit-field-mode-map)
  (setq mode-name "Rec Edit")
  (setq major-mode 'rec-edit-field-mode))

;;; rec-mode.el ends here
