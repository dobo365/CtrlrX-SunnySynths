# BarelyML - Sunny Synths markdown

BarelyML - Sunny Synths markdown is an extension of the BarelyML library made by Friz Menzer in 2023. It is providing a more complete handling of the initialy supported markdwon tags but has also enhanced it with new ones.

Version: 0.8 
Date: June 15th 2026
Author:  Dominique Bontemps - Sunny Synths

- Corrected the usage of table background colours 
- Adapted to be closer to the Github Wiki layout
- Added support for double tilde tag to indicate a strikethrough text
- Handle right/center/left alignment in tables columns
- Links are not in bold; they get a pointing hand cursor when hovered
- Tables are rendered as in Github Wiki (even rows have a very light background, grid in light grey, header in bold) 
- Supports inline code using single back tick tag and code blocks using triple back tick tags
- Code text is displayed in courier new font on a grey rectangle background
- Ability to copy code block by pressing an icon
- Added horizontal rule drawing using standard markdown "---" tag
- Blockquote marking using standard markdown "> " tag
- Adapted Admonition block tagging as "!!! TYPE" and "!!!"
- Possible Admonition block types are: HINT, INFO, CAUTION, WARNING, OTHER, MISC 
- Completely rewritten Admonition blocks rendering that also covers blockquote
- Modified H1 heading rendering to be more in line with GitHub's H1 rendering (with a line drawn below the text)

This file is also used to test and verify all tags.

# Level 1 Heading

## Level 2 Heading

### Level 3 Heading

#### Level 4 Heading

##### Level 5 Heading

This is a standard paragraph. it is used to check the default spacing between text blocks and headings.

This paragraph contains a forced new line jump achieved by inserting
a return at the end of the text line
and is thus directly continuing on this new line.

## Table of contents
* [Level 2 Heading](goto://## Level 2 Heading)
  * [Level 3 Heading](goto://### Level 3 Heading)
* [Styles and Formating](goto://## Styles and Formating)
  * [Lists](goto://### Lists)
  * [Tasks list](goto://### Tasks list)
  * [Blockquote and Code](goto://### Blockquote and Code)
  * [Links and Images](goto://### Links and Images)
  * [Tables](goto://### Tables)
  * [Predefined admonition blocks](goto://### Predefined admonition blocks)
  * [Horizontal rule](goto://### Horizontal rule)
  * [Pure HTML tags](goto://### Pure HTML tags)

## Styles and Formating

Verification and formatting possibilities :
* This text is in **bold**.
* This text is in *italique*.
* This text combines ***bold and italic***.
* This text is in ~~strikethrough~~.

Colors:
* <c:red>Red Text
* <c#AA22FF>#AA22FF color Text

Predefined colors can be used (CGA 16-colour palette with some extensions):
black, blue, green, cyan, red, magenta, brown, lightgray, darkgray, lightblue, lightgreen, lightcyan, lightred, lightmagenta, yellow, white, orange, pink, darkyellow, purple, gray, linkcolour (by default set to blue)

The syntax for a predefined color is <c followed by :color_name>
The syntax for any color is <c followed by #RRGGBB>

### Lists

#### Unordered list
* First bullet point 
* Second bullet point with `inline code`
* Third bullet point with second level list :
    * Second level A
    * Second level B
* Fourth bullet point with code block
```
qsqdqsdsd
```

* Fifth one containing a [Google](www.google.com) link...
* Sixth one...

#### Ordered/Numbered list
1. First mandatory step
2. Second step
3. Third step with code block
```
qsqdqsdsd
```

4. Fourth step

### Tasks list
- [x] Completed task
- [ ] To do...
- [x] Completed task

### Blockquote and Code
> This is `an inline code` within a blockquote.
> A blockquote can have any length. Each line is starting with >

Inline code is identified by a sinlge back tick tag before and after the text while code blocks are using a triple back tick as first and last line.
It is possible to copy the code by clicking on the icon.

Hello `This is an inline code` Hello

```
This is a code block

if xxx then
  qsdqsd
else
  dsqdsqd
end
```

### Links and Images

Vous pouvez insérer un [Lien Externe GitHub Docs](https://github.com) ou ajouter une image avec un texte alternatif :

![Logo Placeholder](https://placeholder.com "Survol")

### Tables

| Identifiant | Nom du Produit | Statut |
| :--- | :---: | ---: |
| #001 | Élément Gauche | En stock |
| #002 | Élément Centré | *Épuisé* |
| #003 | Élément Droite | **Faible** |
| #004 | Élément Centré | ~~Faible~~ |
| #005 | Élément Centré | Go to [Google.com](www.google.com)!  |
| #006 | Élément de code | Go to `code` !  |

### Predefined admonition blocks

!!! INFORMATION
INFO: Hello world
!!!

!!! HINT
HINT: Hello world
!!!

!!! CAUTION
CAUTION: Hello world
!!!

!!! WARNING
WARNING: Hello world
!!!

!!! IMPORTANT
IMPORTANT: Hello world
!!!

!!! OTHER
OTHER: Hello world
!!!

!!! MISC
MISC: Hello world
!!!


### Horizontal rule

---

### Pure HTML tags

<c:red>HTML tags are not supported for the moment