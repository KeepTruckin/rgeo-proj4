/*
  Main initializer for Proj4 wrapper
*/
#ifdef HAVE_PROJ_H
#ifdef HAVE_PROJ_CREATE
#ifdef HAVE_PROJ_CREATE_CRS_TO_CRS_FROM_PJ
#ifdef HAVE_PROJ_NORMALIZE_FOR_VISUALIZATION
#define RGEO_PROJ4_SUPPORTED
#endif
#endif
#endif
#endif

#ifdef HAVE_RB_GC_MARK_MOVABLE
#define mark rb_gc_mark_movable
#else
#define mark rb_gc_mark
#endif

#ifdef __cplusplus
#define RGEO_BEGIN_C extern "C" {
#define RGEO_END_C }
#else
#define RGEO_BEGIN_C
#define RGEO_END_C
#endif


#ifdef RGEO_PROJ4_SUPPORTED

#include <ruby.h>
#include <proj.h>

#endif


RGEO_BEGIN_C


#ifdef RGEO_PROJ4_SUPPORTED

#if PROJ_VERSION_MAJOR == 6 && PROJ_VERSION_MINOR < 3
#define WKT_TYPE PJ_WKT2_2018
#else
#define WKT_TYPE PJ_WKT2_2019
#endif

typedef struct {
  PJ *pj;
  VALUE original_str;
  char uses_radians;
} RGeo_Proj4Data;

typedef struct {
  PJ *crs_to_crs;
} RGeo_CRSToCRSData;


// Destroy function for proj data.
static void rgeo_proj4_free(void *ptr)
{
  RGeo_Proj4Data *data = (RGeo_Proj4Data *)ptr;
  if(data->pj){
    proj_destroy(data->pj);
  }
  free(data);
}

// Destroy function for crs_to_crs data.
static void rgeo_crs_to_crs_free(void *ptr)
{
  RGeo_CRSToCRSData *data = (RGeo_CRSToCRSData *)ptr;
  if(data->crs_to_crs){
    proj_destroy(data->crs_to_crs);
  }
  free(data);
}


static size_t rgeo_proj4_memsize(const void *ptr)
{
  size_t size = 0;
  const RGeo_Proj4Data *data = (const RGeo_Proj4Data *)ptr;

  size += sizeof(*data);
  if(data->pj){
    size += sizeof(data->pj);
  }
  return size;
}

static size_t rgeo_crs_to_crs_memsize(const void *ptr)
{
  size_t size = 0;
  const RGeo_CRSToCRSData *data = (const RGeo_CRSToCRSData *)ptr;
  size += sizeof(*data);
  if(data->crs_to_crs){
    size += sizeof(data->crs_to_crs);
  }
  return size;
}

static void rgeo_proj4_mark(void *ptr)
{
  RGeo_Proj4Data *data = (RGeo_Proj4Data *)ptr;
  if(!NIL_P(data->original_str)){
    mark(data->original_str);
  }
}

#ifdef HAVE_RB_GC_MARK_MOVABLE
static void rgeo_proj4_compact(void *ptr)
{
  RGeo_Proj4Data *data = (RGeo_Proj4Data *)ptr;
  if(data && !NIL_P(data->original_str)){
    data->original_str = rb_gc_location(data->original_str);
  }
}
#endif

static void rgeo_proj4_clear_struct(RGeo_Proj4Data *data)
{
  if(data->pj){
    proj_destroy(data->pj);
    data->pj = NULL;
    data->original_str = Qnil;
  }
}

static const rb_data_type_t rgeo_proj4_data_type = {
    "RGeo::CoordSys::Proj4",
    {rgeo_proj4_mark, rgeo_proj4_free, rgeo_proj4_memsize,
#ifdef HAVE_RB_GC_MARK_MOVABLE
    rgeo_proj4_compact
#endif
    },
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t rgeo_crs_to_crs_data_type = {
    "RGeo::CoordSys::CRSToCRS",
    {0, rgeo_crs_to_crs_free, rgeo_crs_to_crs_memsize},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE rgeo_proj4_data_alloc(VALUE self)
{
  VALUE result;
  RGeo_Proj4Data *data = ALLOC(RGeo_Proj4Data);

  result = Qnil;

  if(data){
    data->pj = NULL;
    data->original_str = Qnil;
    data->uses_radians = 0;
    result = TypedData_Wrap_Struct(self, &rgeo_proj4_data_type, data);
  }
  return result;
}


static VALUE rgeo_crs_to_crs_data_alloc(VALUE self)
{
  VALUE result;
  RGeo_CRSToCRSData *data = ALLOC(RGeo_CRSToCRSData);

  result = Qnil;

  if(data){
    data->crs_to_crs = NULL;
    result = TypedData_Wrap_Struct(self, &rgeo_crs_to_crs_data_type, data);
  }
  return result;
}

static VALUE method_proj4_initialize_copy(VALUE self, VALUE orig)
{
  RGeo_Proj4Data *self_data;
  RGeo_Proj4Data *orig_data;
  const char* str;

  // Clear out any existing value
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, self_data);
  rgeo_proj4_clear_struct(self_data);

  // Copy value from orig
  TypedData_Get_Struct(orig, RGeo_Proj4Data, &rgeo_proj4_data_type, orig_data);
  if (!NIL_P(orig_data->original_str)) {
    self_data->pj = proj_create(PJ_DEFAULT_CTX, StringValuePtr(orig_data->original_str));
  }
  else {
    str = proj_as_proj_string(PJ_DEFAULT_CTX, orig_data->pj, PJ_PROJ_4, NULL);
    self_data->pj = proj_create(PJ_DEFAULT_CTX, str);
  }
  self_data->original_str = orig_data->original_str;
  self_data->uses_radians = orig_data->uses_radians;

  return self;
}


static VALUE method_proj4_set_value(VALUE self, VALUE str, VALUE uses_radians)
{
  RGeo_Proj4Data *self_data;

  Check_Type(str, T_STRING);

  // Clear out any existing value
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, self_data);
  rgeo_proj4_clear_struct(self_data);

  // Set new data
  self_data->pj = proj_create(PJ_DEFAULT_CTX, StringValuePtr(str));
  self_data->original_str = str;
  self_data->uses_radians = RTEST(uses_radians) ? 1 : 0;

  return self;
}


static VALUE method_proj4_get_geographic(VALUE self)
{
  VALUE result;
  RGeo_Proj4Data *new_data;
  RGeo_Proj4Data *self_data;

  result = Qnil;
  new_data = ALLOC(RGeo_Proj4Data);
  if (new_data) {
    TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, self_data);

    new_data->pj = proj_crs_get_geodetic_crs(PJ_DEFAULT_CTX, self_data->pj);
    new_data->original_str = Qnil;
    new_data->uses_radians = self_data->uses_radians;
    result = TypedData_Wrap_Struct(CLASS_OF(self), &rgeo_proj4_data_type, new_data);
  }
  return result;
}


static VALUE method_proj4_original_str(VALUE self)
{
  RGeo_Proj4Data *data;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  return data->original_str;
}


static VALUE method_proj4_uses_radians(VALUE self)
{
  RGeo_Proj4Data *data;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  return data->uses_radians ? Qtrue : Qfalse;
}


static VALUE method_proj4_canonical_str(VALUE self)
{
  VALUE result;
  PJ *pj;
  const char *str;
  RGeo_Proj4Data *data;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  pj = data->pj;
  if (pj) {
    str = proj_as_proj_string(PJ_DEFAULT_CTX, pj, PJ_PROJ_4, NULL);
    if (str) {
      result = rb_str_new2(str);
    }
  }
  return result;
}

static VALUE method_proj4_wkt_str(VALUE self)
{
  VALUE result;
  PJ *pj;
  const char *str;
  RGeo_Proj4Data *data;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  pj = data->pj;
  if (pj) {
    const char *const options[] = {"MULTILINE=NO", NULL};
    str = proj_as_wkt(PJ_DEFAULT_CTX, pj, WKT_TYPE, options);
    if(str){
      result = rb_str_new2(str);
    }
  }
  return result;
}

static VALUE method_proj4_auth_name_str(VALUE self)
{
  VALUE result;
  PJ *pj;
  const char *id;
  const char *auth;
  RGeo_Proj4Data *data;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  pj = data->pj;
  if (pj) {
    auth = proj_get_id_auth_name(pj, 0);
    id = proj_get_id_code(pj, 0);
    if(id && auth){
      result = rb_sprintf("%s:%s", auth, id);
    }
  }
  return result;
}

static VALUE method_proj4_is_geographic(VALUE self)
{
  VALUE result;
  PJ *pj;
  PJ_TYPE proj_type;
  RGeo_Proj4Data *data;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  pj = data->pj;
  if (pj) {
    proj_type = proj_get_type(pj);
    if(proj_type == PJ_TYPE_GEOGRAPHIC_2D_CRS || proj_type == PJ_TYPE_GEOGRAPHIC_3D_CRS){
      result = Qtrue;
    } else {
      result = Qfalse;
    }
  }
  return result;
}


static VALUE method_proj4_is_geocentric(VALUE self)
{
  VALUE result;
  PJ *pj;
  PJ_TYPE proj_type;
  RGeo_Proj4Data *data;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  pj = data->pj;
  if (pj) {
    proj_type = proj_get_type(pj);
    result = proj_type == PJ_TYPE_GEOCENTRIC_CRS ? Qtrue : Qfalse;
  }
  return result;
}


static VALUE method_proj4_is_valid(VALUE self)
{
  RGeo_Proj4Data *data;
  TypedData_Get_Struct(self, RGeo_Proj4Data, &rgeo_proj4_data_type, data);
  return data->pj ? Qtrue : Qfalse;
}


static VALUE cmethod_proj4_version(VALUE module)
{
  return rb_sprintf("%d.%d.%d", PROJ_VERSION_MAJOR, PROJ_VERSION_MINOR, PROJ_VERSION_PATCH);
}

static VALUE cmethod_proj4_create(VALUE klass, VALUE str, VALUE uses_radians)
{
  VALUE result;
  RGeo_Proj4Data* data;

  result = Qnil;
  Check_Type(str, T_STRING);
  data = ALLOC(RGeo_Proj4Data);
  if (data) {
    data->pj = proj_create(PJ_DEFAULT_CTX, StringValuePtr(str));
    data->original_str = str;
    data->uses_radians = RTEST(uses_radians) ? 1 : 0;
    result = TypedData_Wrap_Struct(klass, &rgeo_proj4_data_type, data);
  }
  return result;
}

static VALUE cmethod_crs_to_crs_create(VALUE klass, VALUE from, VALUE to)
{
  VALUE result;
  RGeo_Proj4Data *from_data;
  RGeo_Proj4Data *to_data;
  result = Qnil;
  PJ *from_pj;
  PJ *to_pj;
  PJ *gis_pj;
  PJ *crs_to_crs;
  RGeo_CRSToCRSData* data;

  TypedData_Get_Struct(from, RGeo_Proj4Data, &rgeo_proj4_data_type, from_data);
  TypedData_Get_Struct(to, RGeo_Proj4Data, &rgeo_proj4_data_type, to_data);
  from_pj = from_data->pj;
  to_pj = to_data->pj;
  crs_to_crs = proj_create_crs_to_crs_from_pj(PJ_DEFAULT_CTX, from_pj, to_pj, 0, NULL);

  // necessary to use proj_normalize_for_visualization so that we
  // do not have to worry about the order of coordinates in every
  // coord system
  gis_pj = proj_normalize_for_visualization(PJ_DEFAULT_CTX, crs_to_crs);
  if(gis_pj){
    proj_destroy(crs_to_crs);
    crs_to_crs = gis_pj;
  }
  data = ALLOC(RGeo_CRSToCRSData);
  if (data){
    data->crs_to_crs = crs_to_crs;
    result = TypedData_Wrap_Struct(klass, &rgeo_crs_to_crs_data_type, data);
  }
  return result;
}


static VALUE method_crs_to_crs_transform(VALUE self, VALUE x, VALUE y, VALUE z)
{
  VALUE result;
  RGeo_CRSToCRSData *crs_to_crs_data;
  PJ *crs_to_crs_pj;
  double xval, yval, zval;
  PJ_COORD input;
  PJ_COORD output;

  result = Qnil;
  TypedData_Get_Struct(self, RGeo_CRSToCRSData, &rgeo_crs_to_crs_data_type, crs_to_crs_data);
  crs_to_crs_pj = crs_to_crs_data->crs_to_crs;
  if(crs_to_crs_pj){
    xval = rb_num2dbl(x);
    yval = rb_num2dbl(y);
    zval = NIL_P(z) ? 0.0 : rb_num2dbl(z);

    input = proj_coord(xval, yval, zval, HUGE_VAL);
    output = proj_trans(crs_to_crs_pj, PJ_FWD, input);

    result = rb_ary_new2(NIL_P(z) ? 2 : 3);
    rb_ary_push(result, DBL2NUM(output.xyz.x));
    rb_ary_push(result, DBL2NUM(output.xyz.y));
    if(!NIL_P(z)){
      rb_ary_push(result, DBL2NUM(output.xyz.z));
    }
  }
  return result;
}

static void rgeo_init_proj4()
{
  VALUE rgeo_module;
  VALUE coordsys_module;
  VALUE proj4_class;
  VALUE crs_to_crs_class;

  rgeo_module = rb_define_module("RGeo");
  coordsys_module = rb_define_module_under(rgeo_module, "CoordSys");

  proj4_class = rb_define_class_under(coordsys_module, "Proj4", rb_cObject);
  rb_define_alloc_func(proj4_class, rgeo_proj4_data_alloc);
  rb_define_module_function(proj4_class, "_create", cmethod_proj4_create, 2);
  rb_define_method(proj4_class, "initialize_copy", method_proj4_initialize_copy, 1);
  rb_define_method(proj4_class, "_set_value", method_proj4_set_value, 2);
  rb_define_method(proj4_class, "_original_str", method_proj4_original_str, 0);
  rb_define_method(proj4_class, "_canonical_str", method_proj4_canonical_str, 0);
  rb_define_method(proj4_class, "_as_text", method_proj4_wkt_str, 0);
  rb_define_method(proj4_class, "_auth_name", method_proj4_auth_name_str, 0);
  rb_define_method(proj4_class, "_valid?", method_proj4_is_valid, 0);
  rb_define_method(proj4_class, "_geographic?", method_proj4_is_geographic, 0);
  rb_define_method(proj4_class, "_geocentric?", method_proj4_is_geocentric, 0);
  rb_define_method(proj4_class, "_radians?", method_proj4_uses_radians, 0);
  rb_define_method(proj4_class, "_get_geographic", method_proj4_get_geographic, 0);
  rb_define_module_function(proj4_class, "_proj_version", cmethod_proj4_version, 0);


  crs_to_crs_class = rb_define_class_under(coordsys_module, "CRSToCRS", rb_cObject);
  rb_define_alloc_func(crs_to_crs_class, rgeo_crs_to_crs_data_alloc);
  rb_define_module_function(crs_to_crs_class, "_create", cmethod_crs_to_crs_create, 2);
  rb_define_method(crs_to_crs_class, "_transform_coords", method_crs_to_crs_transform, 3);
}


#endif


void Init_proj4_c_impl()
{
#ifdef RGEO_PROJ4_SUPPORTED
  rgeo_init_proj4();
#endif
}


RGEO_END_C
