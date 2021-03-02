(define-expansion (set-boolopts . opts)
                  `(map s9-option-set-boolopt ',opts))
(define-expansion (add-menucolors . c)
                  `(map (lambda (k v)
                            (if (list? k)
                              (map (lambda (k) (s9-option-add-menucolor k v)) k)
                              (s9-option-add-menucolor k v))) ',(map car c) (list ,@(map cadr c))))
(define add-menucolours add-menucolors)


(define (s9-style? x)
  (let ((style-names '(black red green brown blue magenta cyan gray orange lightgreen yellow lightblue lightmagenta lightcyan white
                             bold blink italic inverse underline)))
    (cond
      ((symbol? x) (member x style-names))
      ((string? x) (s9-style? (string->symbol x)))
      ((list? x) (apply and (map s9-style? x)))
      (#t #f))))

(define *blessed-style* 'green)
(define *cursed-style* 'red)
(define *cursed-worn-style* '(orange underline))
(define *holy-water-style* '(cyan bold))
(define *unholy-water-style* 'orange)
(define *gold-style* 'yellow)
(define *loadstone-style* '(red underline))
(define *goody-style* 'magenta)

(define *magic-armor-style* #f)
(define *magic-armour-style* 'magenta)

(define *unid-magic-armor-style* #f)
(define *unid-magic-armour-style* #f)
(define *unid-magic-cloak-style* #f)
(define *unid-magic-helm-style* #f)
(define *unid-magic-glove-style* #f)
(define *unid-magic-boot-style* #f)
(define *id-magic-armor-style* #f)
(define *id-magic-armour-style* #f)
(define *id-magic-cloak-style* #f)
(define *id-magic-helm-style* #f)
(define *id-magic-glove-style* #f)
(define *id-magic-boot-style* #f)
(define *good-magic-armor-style* #f)
(define *good-magic-armour-style* #f)
(define *good-magic-cloak-style* #f)
(define *good-magic-helm-style* #f)
(define *good-magic-glove-style* #f)
(define *good-magic-boot-style* #f)

(define (add-default-menucolors)
  (let* ((blessed-style *blessed-style*)
         (cursed-style *cursed-style*)
         (cursed-worn-style (or *cursed-worn-style* cursed-style))
         (holy-water-style (or *holy-water-style* blessed-style))
         (unholy-water-style (or *unholy-water-style* cursed-worn-style))

         (gold-style *gold-style*)
         (loadstone-style *loadstone-style*)
         (goody-style *goody-style*)

         (magic-armor-style (or *magic-armor-style* *magic-armour-style*))
         (unid-magic-armor-style (or *unid-magic-armor-style* *unid-magic-armour-style* magic-armor-style))
         (unid-magic-cloak-style (or *unid-magic-cloak-style* unid-magic-armor-style))
         (unid-magic-helm-style (or *unid-magic-helm-style* unid-magic-armor-style))
         (unid-magic-glove-style (or *unid-magic-glove-style* unid-magic-armor-style))
         (unid-magic-boot-style (or *unid-magic-boot-style* unid-magic-armor-style))

         (id-magic-armor-style (or *id-magic-armor-style* *id-magic-armour-style* magic-armor-style))
         (id-magic-cloak-style (or *id-magic-cloak-style* id-magic-armor-style))
         (id-magic-helm-style (or *id-magic-helm-style* id-magic-armor-style))
         (id-magic-glove-style (or *id-magic-glove-style* id-magic-armor-style))
         (id-magic-boot-style (or *id-magic-boot-style* id-magic-armor-style))

         (good-magic-armor-style (or *good-magic-armor-style* *good-magic-armour-style* magic-armor-style))
         (good-magic-cloak-style (or *good-magic-cloak-style* good-magic-armor-style))
         (good-magic-helm-style (or *good-magic-helm-style* good-magic-armor-style))
         (good-magic-glove-style (or *good-magic-glove-style* good-magic-armor-style))
         (good-magic-boot-style (or *good-magic-boot-style* good-magic-armor-style)))

    (add-menucolours
      ; beatitude
      ("blessed" blessed-style)
      ("\\bcursed" cursed-style)
      ("\\bcursed .* \\(being worn\\)" cursed-worn-style)
      (("holy water"
        "blessed clear potion"
        "blessed potions? called water"
        "clear potions? named (holy|blessed)"
        "potions (of|called) water named (holy|blessed)") holy-water-style)
      (("unholy water"
        "\\bcursed clear potion"
        "\\bcursed potions? called water"
        "potions? (of|called) water named (unholy|cursed)") unholy-water-style)

      ("gold piece" gold-style)

      ("load(stone)?" loadstone-style)

      ; goodies
      (("bag .* holding"
        "luck(stone)?"
        "wand .* wishing?"
        "wand .* teleport(ation)?"
        "wand .* polymorph"
        "wand .* death"
        "gain level"
        "full healing"
        "magic marker"
        "magic lamp|lamp .* magic"
        "unicorn horn[^[]*$" ; this doesn't colour the #enhance unicorn
        "tinning kit"
        "ring .* regeneration"
        "ring .* conflict"
        "ring .* free action"
        "ring .* teleport control"
        "ring .* levitation"
        "scrolls? .* genocide"
        "scrolls? .* charging"
        "scrolls? .* identify"
        "amulet .* life ?saving"
        "amulet .* reflection"
        "c(o|hi)ckatrice (corpse|egg)"
        "egg .* cockatrice"
        "stethoscope") goody-style)

      ; magical armour (unidentified)
      (("tattered cape"
        "dirty rag"
        "opera cloak"
        "ornamental cope"
        "piece of cloth") unid-magic-cloak-style)
      (("plumed helmet"
        "etched helmet"
        "crested helmet"
        "visored helmet") unid-magic-helm-style)
      ("(padded|riding|black|fencing) gloves" unid-magic-glove-style)
      ("(combat|jungle|hiking|mud|steel|riding|snow) boots" unid-magic-boot-style)

      ; " (identified)
      ("cloak of (protection|invisibility|magic resistance|displacement)" id-magic-cloak-style)
      ("(helm of|helmet called) (brilliance|opposite alignment|telepathy)" id-magic-helm-style)
      ("(gauntlets of|gloves called) (power|swimming|dexterity)" id-magic-glove-style)
      (("(speed|water walking|jumping|elven|kicking|levitation) boots"
        "boots called (speed|water walking|jumping|elven|kicking|levitation)") id-magic-boot-style)

      ; good magical armour (identified)
      ("cloak of magic resistance" good-magic-cloak-style)
      ("(helm of|helmet called) brilliance" good-magic-helm-style)
      ("(gauntlets of|gloves called) (power|dexterity)" good-magic-glove-style)
      ("speed boots|boots called speed" good-magic-boot-style))))
