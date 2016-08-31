Assets
======

Managing and using graphical assets in VBDE
-------------------------------------------

VBDE allows for hassle free, on-the-fly image conversion using grit. To use it, create a folder called "assets" in the root folder of your project, with a contained "images" folder. This folder will hold all your images in any of the supported formats: png, bmp, gif, pcx or jpg/jpeg (I recommend png.)

Images should use an indexed 4-color palette in the following order: Black, Dark Red, Medium Red, Light Red. An example palette for Photoshop can be found in {VBDE}/system/grit/.

In order to be detected and converted by the image conversion script, you also need a *.grit file of the same name as the graphic, which contains the conversion settings. You can find a number of example grit files in {VBDE}/system/grit/ which you can just copy and paste (and rename) into your images folder.

You can also make several files convert at once instead of having all images converted individually. Grit files which do not directly relate to an image file through their name, are applied to all images files in the current directory (not sub-directories.) This is useful for generating maps with shared tilesets.

Example file structure:
	my-project/
	  '- assets/
	    '- images/
	      |- backgrounds/
          | |- Backgrounds.grit
          | |- Background-1.png
          | '- Background-2.png
          |- Font.grit
          |- Font.png
          |- Hero.grit
          '- Hero.png

Finally, to start the image conversion, press the editor's grit button while having any project file opened. This will convert all images that do not yet have a corresponding c file or are newer than their c file. 