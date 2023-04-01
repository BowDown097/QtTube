#nullable disable
// ASP.NET YouTube annotations proxy - used at https://sperg.rodeo/qttube_annotations

public static class QtTubeAnnotations
{
    private const string AnnotationsEndpoint = "https://storage.googleapis.com/biggest_bucket/annotations";

    private static string GetVideoPath(string videoId)
    {
        string annotationFileDirectory = videoId[0].ToString();
        if (annotationFileDirectory == "-")
            annotationFileDirectory = "-/ar-";

        return $"{AnnotationsEndpoint}/{annotationFileDirectory}/{videoId[..3]}/{videoId}.xml.gz";
    }

    public static async Task Execute(HttpContext context)
    {
        context.Response.Headers.Add("Access-Control-Allow-Origin", "*");

        string videoId = context.Request.Query["videoId"];
        if (string.IsNullOrWhiteSpace(videoId))
        {
            context.Response.StatusCode = 400;
            await context.Response.WriteAsync("Failed: missing videoId");
            return;
        }

        string requestUrl = GetVideoPath(videoId);
        using HttpClient client = new();
        string annotationsData = await client.GetStringAsync(requestUrl);

        if (string.IsNullOrWhiteSpace(annotationsData))
        {
            context.Response.StatusCode = 400;
            await context.Response.WriteAsync("Failed: Annotation data is unavailable for this video");
            return;
        }

        await context.Response.WriteAsync(annotationsData);
    }
}
