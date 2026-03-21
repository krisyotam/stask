/* stask - simple task manager
 * See LICENSE file for copyright and license details. */

/* krisyotam.com monochromatic palette */
/* all colors: {r, g, b, a} - pure grayscale (0 saturation) */

static const Theme theme_light = {
	/* bg:           hsl(0 0% 98%) */
	.bg         = {0.98, 0.98, 0.98, 1.0},
	/* card:         hsl(0 0% 99%) */
	.card       = {0.99, 0.99, 0.99, 1.0},
	/* fg:           hsl(0 0% 20%) */
	.fg         = {0.20, 0.20, 0.20, 1.0},
	/* border:       hsl(0 0% 88%) */
	.border     = {0.88, 0.88, 0.88, 1.0},
	/* muted:        hsl(0 0% 92%) */
	.muted      = {0.92, 0.92, 0.92, 1.0},
	/* muted-fg:     hsl(0 0% 50%) */
	.muted_fg   = {0.50, 0.50, 0.50, 1.0},
	/* primary:      hsl(0 0% 10%) */
	.primary    = {0.10, 0.10, 0.10, 1.0},
	/* primary-fg:   hsl(0 0% 95%) */
	.primary_fg = {0.95, 0.95, 0.95, 1.0},
	/* selection:    hsl(0 0% 40%) */
	.sel        = {0.40, 0.40, 0.40, 1.0},
	/* accent:       hsl(0 0% 10%) - dark in light mode */
	.accent     = {0.10, 0.10, 0.10, 1.0},
	/* accent-fg:    hsl(0 0% 95%) */
	.accent_fg  = {0.95, 0.95, 0.95, 1.0},
};

static const Theme theme_dark = {
	/* bg:           hsl(0 0% 7%) */
	.bg         = {0.07, 0.07, 0.07, 1.0},
	/* card:         hsl(0 0% 9%) */
	.card       = {0.09, 0.09, 0.09, 1.0},
	/* fg:           hsl(0 0% 98%) */
	.fg         = {0.98, 0.98, 0.98, 1.0},
	/* border:       hsl(0 0% 15%) */
	.border     = {0.15, 0.15, 0.15, 1.0},
	/* muted:        hsl(0 0% 12%) */
	.muted      = {0.12, 0.12, 0.12, 1.0},
	/* muted-fg:     hsl(0 0% 70%) */
	.muted_fg   = {0.70, 0.70, 0.70, 1.0},
	/* primary:      hsl(0 0% 98%) */
	.primary    = {0.98, 0.98, 0.98, 1.0},
	/* primary-fg:   hsl(0 0% 9%) */
	.primary_fg = {0.09, 0.09, 0.09, 1.0},
	/* selection:    hsl(0 0% 83%) */
	.sel        = {0.83, 0.83, 0.83, 1.0},
	/* accent:       hsl(0 0% 88%) - light in dark mode */
	.accent     = {0.88, 0.88, 0.88, 1.0},
	/* accent-fg:    hsl(0 0% 9%) */
	.accent_fg  = {0.09, 0.09, 0.09, 1.0},
};

/* default: 0=light, 1=dark */
static const int default_dark = 0;

/* GTK CSS for squared corners (krisyotam.com aesthetic) */
static const char *app_css =
	"* { border-radius: 0; }\n"
	"button { border-radius: 0; padding: 4px 10px; }\n"
	"popover > contents { border-radius: 0; padding: 0; }\n"
	"entry { border-radius: 0; }\n"
	"headerbar { border-radius: 0; }\n"
	"window { border-radius: 0; }\n"
	"separator { margin: 0 2px; }\n"
	"checkbutton indicator { border-radius: 0; }\n";
