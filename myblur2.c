#include <libgimp/gimp.h>

static void query (void);
static void run   (const gchar      *name,
                   gint              nparams,
                   const GimpParam  *param,
                   gint             *nreturn_vals,
                   GimpParam       **return_vals);
static void blur  (GimpDrawable     *drawable);

GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,
  NULL,
  query,
  run
};

MAIN()

static void
query (void)
{
  static GimpParamDef args[] =
  {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Run mode"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Input image"
    },
    {
      GIMP_PDB_DRAWABLE,
      "drawable",
      "Input drawable"
    }
  };

  gimp_install_procedure (
    "plug-in-checkboard",
    "Checkboard",
    "Creates a checkboard",
    "Varsha & Srihari",
    "Copyright Varsha & Srihari",
    "2012",
    "_Checkboard",
    "RGB*, GRAY*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args), 0,
    args, NULL);

  gimp_plugin_menu_register ("plug-in-checkboard",
                             "<Image>/Filters/My Plugins");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam  values[1];
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  GimpRunMode       run_mode;
  GimpDrawable     *drawable;

  /* Setting mandatory output values */
  *nreturn_vals = 1;
  *return_vals  = values;

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;

  /* Getting run_mode - we won't display a dialog if
   * we are in NONINTERACTIVE mode
   */
  run_mode = param[0].data.d_int32;

  /*  Get the specified drawable  */
  drawable = gimp_drawable_get (param[2].data.d_drawable);

  gimp_progress_init ("My Blur...");

  /* Let's time blur
   *
   *   GTimer timer = g_timer_new time ();
   */

  blur (drawable);

  /*   g_print ("blur() took %g seconds.\n", g_timer_elapsed (timer));
   *   g_timer_destroy (timer);
   */

  gimp_displays_flush ();
  gimp_drawable_detach (drawable);

  return;
}

static void
blur (GimpDrawable *drawable)
{
  gint         i, j, k, channels;
  gint         x1, y1, x2, y2;
  GimpPixelRgn rgn_in, rgn_out;
  guchar      *row1, *row2, *row3;
  guchar      *outrow;

  gimp_drawable_mask_bounds (drawable->drawable_id,
                             &x1, &y1,
                             &x2, &y2);
  channels = gimp_drawable_bpp (drawable->drawable_id);

  gimp_pixel_rgn_init (&rgn_in,
                       drawable,
                       x1, y1,
                       x2 - x1, y2 - y1,
                       FALSE, FALSE);
  gimp_pixel_rgn_init (&rgn_out,
                       drawable,
                       x1, y1,
                       x2 - x1, y2 - y1,
                       TRUE, TRUE);

  /* Initialise enough memory for row1, row2, row3, outrow */
  row1 = g_new (guchar, channels * (x2 - x1));
  row2 = g_new (guchar, channels * (x2 - x1));
  row3 = g_new (guchar, channels * (x2 - x1));
  outrow = g_new (guchar, channels * (x2 - x1));

  for (i = y1; i < y2; i++)
    {
      gint colwid = (x2-x1)/8;
      gboolean isColored = FALSE;
      for (j = x1; j < x2; j++)
        {
          /* For each layer, compute the average of the nine
           * pixels */

          if(j%colwid == 0) isColored = !isColored;
          if(!isColored) continue;

          for (k = 0; k < channels; k++)
            outrow[channels * (j - x1) + k] = 66;
        }

      gimp_pixel_rgn_set_row (&rgn_out,
                              outrow,
                              x1, i,
                              x2 - x1);

      if (i % 10 == 0)
        gimp_progress_update ((gdouble) (i - y1) / (gdouble) (y2 - y1));
    }

  g_free (row1);
  g_free (row2);
  g_free (row3);
  g_free (outrow);

  gimp_drawable_flush (drawable);
  gimp_drawable_merge_shadow (drawable->drawable_id, TRUE);
  gimp_drawable_update (drawable->drawable_id,
                        x1, y1,
                        x2 - x1, y2 - y1);
}

