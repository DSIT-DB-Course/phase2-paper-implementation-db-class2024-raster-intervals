#include "../../include/join_algos/inter_filter.h"
#include "../../include/utils/geometry_types.h"
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>


void save_vertices(std::vector<const Point *> vertices, const char *output_file,
                   const char *mode) {
  FILE *fp = fopen(output_file, mode);
  for (int i = 0; i < vertices.size(); ++i) {
    fprintf(fp, "%lf %lf\n", vertices[i]->x, vertices[i]->y);
  }
  fclose(fp);
}

void save_vertices_vectors(
    const char *output_file,
    std::vector<std::vector<const Point *>> &vertices_vectors,
    const char *mode) {

  FILE *fp = fopen(output_file, mode);

  for (int i = 0; i < vertices_vectors.size(); i++) {
    std::vector<const Point *> vec = vertices_vectors[i];
    for (const Point *point : vec) {
      fprintf(fp, "%lf %lf\n", point->x, point->y);
    }

    fprintf(fp, "poly\n");
  }
  fclose(fp);
}

void save_vertices_vectors_seg(std::vector<std::vector<const Point *>> vertices_vectors,
                     const Point &p1, const Point &p2,
                     const char *output_file) {

  FILE *fp = fopen(output_file, "w");
  fprintf(fp, "segment %lf %lf %lf %lf\n", p1.x, p1.y, p2.x, p2.y);
  fclose(fp); 
  for (const std::vector<const Point *> &vertices : vertices_vectors) {
    save_vertices(vertices, output_file, "a");
  }

}


void save_clipped_vertices(
    const char *output_file,
    const std::map<std::pair<unsigned int, unsigned int>,
                   const std::vector<const Point *>> &i_j_to_clipped_vertices,
    const char *mode) {

  FILE *fp = fopen(output_file, mode);
  for (const auto &entry : i_j_to_clipped_vertices) {
    for (const Point *point : entry.second) {
      fprintf(fp, "%lf %lf\n", point->x, point->y);
    }
    fprintf(fp, "poly %d %d\n", entry.first.first, entry.first.second);
  }
  fclose(fp);
}


void save_clipped_vertices_cell_type(const char *output_file,
                        const std::map<std::pair<unsigned int, unsigned int>,
                                       RasterCellInfo> &i_j_to_rcell_info,
                        const char *mode) {

  FILE *fp = fopen(output_file, mode);
  for (const auto &entry : i_j_to_rcell_info) {
    unsigned int i = entry.first.first;
    unsigned int j = entry.first.second;
    const RasterCellInfo &rcell_info = entry.second;

    for (const std::vector<const Point *> &vec : rcell_info.cell_polygons) {
      for (const Point *point : vec) {
        fprintf(fp, "%lf %lf\n", point->x, point->y);
      }

      fprintf(fp, "poly\n");
    }

    fprintf(fp, "cell %d %d\n", i, j);
    fprintf(fp, "%s\n", rcell_info.cell_type.to_type());
  }
  fclose(fp);
}


bool RasterGrid::save_poly_raster(const char *output_file, Polygon& polygon, 
   std::map<std::pair<unsigned int, unsigned int>, RasterCellInfo> &i_j_to_rcell_info, const char *mode) {
    
  double xmin =
      max_below_or_equal_k(min_corner.x, max_corner.x, polygon.minCorner.x);
  double ymin =
      max_below_or_equal_k(min_corner.y, max_corner.y, polygon.minCorner.y);
  double xmax =
      min_above_or_equal_k(min_corner.x, max_corner.x, polygon.maxCorner.x);
  double ymax =
      min_above_or_equal_k(min_corner.y, max_corner.y, polygon.maxCorner.y);

  if (std::isnan(xmin) || std::isnan(ymin) || std::isnan(xmax) ||
      std::isnan(ymax)) {
    std::cerr << "Error in save_poly_raster: min_above_or_equal_k or "
                 "max_below_or_equal_k returned NaN\n";
    return false;
  }

  int xmin_idx = sequence_idx(xmin, min_corner.x);
  int xmax_idx = sequence_idx(xmax, min_corner.x);

  int ymin_idx = sequence_idx(ymin, min_corner.y);
  int ymax_idx = sequence_idx(ymax, min_corner.y);

  FILE *fp = fopen(output_file, mode);
  fprintf(fp, "%d %d %d %d\n", xmin_idx, ymin_idx, xmax_idx, ymax_idx);
  fprintf(fp, "%lf %lf %lf %lf %lf\n", min_corner.x, min_corner.y, max_corner.x, max_corner.y, step);
  
  for (const auto &entry : i_j_to_rcell_info) {
    unsigned int j = entry.first.first;
    unsigned int i = entry.first.second;
    const BinaryCellCode &cell_code = entry.second.cell_type;

    fprintf(fp, "%d %d %s\n", i , j , cell_code.to_type());
  }
  fclose(fp);

  polygon.save_poly(output_file, "a");

  return true;
}


//
// TO BE IMPLEMENTED
// IS NOT COMPLETED
bool RasterGrid::save_polygons_grid(const char *output_file, std::vector<Polygon>& polygons, 
   std::map<std::pair<unsigned int, unsigned int>, RasterCellInfo> &i_j_to_rcell_info, const char *mode) {

  FILE *fp = fopen(output_file, mode);

  fprintf(fp, "%lf %lf %lf %lf %lf\n", min_corner.x, min_corner.y, max_corner.x, max_corner.y, step);
  for (const auto &entry : i_j_to_rcell_info) {
      unsigned int j = entry.first.first;
      unsigned int i = entry.first.second;
      const BinaryCellCode &cell_code = entry.second.cell_type;

      fprintf(fp, "%d %d %s\n", i , j , cell_code.to_type());
  }

  for (Polygon& polygon : polygons){
    double xmin =
        max_below_or_equal_k(min_corner.x, max_corner.x, polygon.minCorner.x);
    double ymin =
        max_below_or_equal_k(min_corner.y, max_corner.y, polygon.minCorner.y);
    double xmax =
        min_above_or_equal_k(min_corner.x, max_corner.x, polygon.maxCorner.x);
    double ymax =
        min_above_or_equal_k(min_corner.y, max_corner.y, polygon.maxCorner.y);

    if (std::isnan(xmin) || std::isnan(ymin) || std::isnan(xmax) ||
        std::isnan(ymax)) {
      std::cerr << "Error in weiler_rasterize_poly->save_polygons_grid: min_above_or_equal_k or "
                  "max_below_or_equal_k returned NaN\n";
      return false;
    }

    int xmin_idx = sequence_idx(xmin, min_corner.x);
    int xmax_idx = sequence_idx(xmax, min_corner.x);

    int ymin_idx = sequence_idx(ymin, min_corner.y);
    int ymax_idx = sequence_idx(ymax, min_corner.y);
    fprintf(fp, "%d %d %d %d\n", xmin_idx, ymin_idx, xmax_idx, ymax_idx);
  
    polygon.save_poly(output_file, "a");
    fprintf(fp, "POLYGON\n");
  }
  fclose(fp);

  return true;
}
